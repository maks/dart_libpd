import 'package:dart_libpd/libpd.dart';

void main(List<String> args) {
  final filename = args[0];
  final path = args[1];
  print('Running libPD test: $filename $path');

  testRunLpd(filename, path);
}
