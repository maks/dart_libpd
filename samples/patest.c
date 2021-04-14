/* C example of using libpd with pulseaudio
 * RECORD & PLAYBACK
 *
 * gcc -c  main.c `pkg-config --cflags libpulse-simple`  -o main.o "-I." "-I/home/psc/src/libpd/libpd_wrapper" "-I/home/psc/src/libpd/pure-data/src"
 * gcc -o myapp main.o  "-L." -O2 `pkg-config --libs libpulse-simple` /home/psc/src/libpd/libs/libpd.so
 *
 * https://www.workinprogress.ca/libpd
 */
#include <stdio.h>

// libpd
#include "z_libpd.h"

// pulseaudio
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#define NUMBERSECOND 10
#define SAMPLE_RATE 44100
#define NBCHANNEL 2
#define BLOCKSIZE 64
#define TICK 1

//libpd catch [print] function
void pdprint(const char *s)
{
    printf("%s", s);
}

int main(int argc, char **argv)
{

    if (argc < 3)
    {
        fprintf(stderr, "usage: %s file folder\nexample: ./libpd echo.pd ./\n", argv[0]);
        return -1;
    }

    // libpd
    libpd_set_printhook(pdprint);
    libpd_init();
    libpd_init_audio(NBCHANNEL, NBCHANNEL, SAMPLE_RATE); // input channel, output channel, sr, tick per buffer
    // compute audio    [; pd dsp 1(
    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");
    // open patch       [; pd open file folder(
    libpd_start_message(2);
    libpd_add_symbol(argv[1]);
    libpd_add_symbol(argv[2]);
    libpd_finish_message("pd", "open");

    // pulseaudio
    // puredata use PA_SAMPLE_FLOAT32LE
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_FLOAT32LE,
        .rate = SAMPLE_RATE,
        .channels = NBCHANNEL,
    };
    // low-latency setting
    pa_buffer_attr ba;
    ba.tlength = pa_usec_to_bytes(50 * 1000, &ss);
    ba.minreq = pa_usec_to_bytes(0, &ss);
    ba.maxlength = pa_usec_to_bytes(50 * 1000, &ss);
    ba.fragsize = sizeof(uint32_t) - 1; //Adjust this value if it's glitchy
    pa_simple *s = NULL;
    pa_simple *r = NULL;
    // init pulseaudio
    int error;
    if (!(r = pa_simple_new(NULL, "pdlib", PA_STREAM_RECORD, NULL, argv[1], &ss, NULL, &ba, &error)))
    {
        fprintf(stderr, __FILE__ ": pa_simple_new() - record failed: %s\n", pa_strerror(error));
        goto problem;
    }
    if (!(s = pa_simple_new(NULL, "pdlib", PA_STREAM_PLAYBACK, NULL, argv[1], &ss, NULL, &ba, &error)))
    {
        fprintf(stderr, __FILE__ ": pa_simple_new() - playback failed: %s\n", pa_strerror(error));
        goto problem;
    }
    // now run pd for ten seconds (logical time)
    int i;
    float inbuf[NBCHANNEL * BLOCKSIZE * TICK], outbuf[NBCHANNEL * BLOCKSIZE * TICK];
    for (i = 0; i < NUMBERSECOND * SAMPLE_RATE / (BLOCKSIZE * TICK); i++)
    {
        // pulseaudio input
        if (pa_simple_read(r, inbuf, sizeof(t_float) * BLOCKSIZE * NBCHANNEL * TICK, &error) < 0)
        {
            fprintf(stderr, __FILE__ ": pa_simple_read() failed: %s\n", pa_strerror(error));
            goto problem;
        }
        // libpd process dsp
        libpd_process_float(1, inbuf, outbuf);
        // pulseaudio output
        if (pa_simple_write(s, outbuf, sizeof(t_float) * BLOCKSIZE * NBCHANNEL * TICK, &error) < 0)
        {
            fprintf(stderr, __FILE__ ": pa_simple_write() failed: %s\n", pa_strerror(error));
            goto problem;
        }
        /*
		//pulseaudio - report latency
		pa_usec_t latency;
		if ((latency = pa_simple_get_latency(s, &error)) == (pa_usec_t) -1) {
		    fprintf(stderr, __FILE__": pa_simple_get_latency() failed: %s\n", pa_strerror(error));
		    goto problem;
		}
		fprintf(stderr, "%0.0f usec    \r", (float)latency);
		*/
    }
    /* Make sure that every single sample was played */
    if (pa_simple_drain(s, &error) < 0)
    {
        fprintf(stderr, __FILE__ ": pa_simple_drain() failed: %s\n", pa_strerror(error));
        goto problem;
    }
    pa_simple_free(s);
    pa_simple_free(r);
    return 0;
problem:
    if (s)
    {
        pa_simple_free(s);
    }
    if (r)
    {
        pa_simple_free(r);
    }
    fprintf(stderr, "An error occured");
    return -1;
}