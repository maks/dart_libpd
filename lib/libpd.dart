import 'dart:ffi';
import 'package:ffi/ffi.dart';

import 'generated_bindings.dart';

final lpd = NativeLibrary(DynamicLibrary.open('libpd/libpd.so'));

void testRunLpd(String filename, String dirPath) {
  final numberOfSeconds = 10;
  final sampleRate = 44100;
  final outChannels = 2;
  final inChannels = 2;

  lpd.libpd_init();

  // input channel, output channel, sr, tick per buffer
  lpd.libpd_init_audio(inChannels, outChannels, sampleRate);

  // https://github.com/dart-lang/ffigen/issues/72#issuecomment-672060509
  final receiver = 'pd'.toNativeUtf8().cast<Int8>();
  final mesg = 'dsp'.toNativeUtf8().cast<Int8>();

  // compute audio    [; pd dsp 1(
  lpd.libpd_start_message(1);
  lpd.libpd_add_float(1.0);
  lpd.libpd_finish_message(receiver, mesg);

  final pdFileName = filename.toNativeUtf8().cast<Int8>();
  final pdDirPath = dirPath.toNativeUtf8().cast<Int8>();

  // open patch       [; pd open file folder(
  final mesg2 = 'open'.toNativeUtf8().cast<Int8>();
  lpd.libpd_start_message(2);
  lpd.libpd_add_symbol(pdFileName);
  lpd.libpd_add_symbol(pdDirPath);
  lpd.libpd_finish_message(receiver, mesg2);
}
