import 'dart:ffi';

typedef NativeC = Int32 Function(
  Pointer<Uint8> input, Pointer<Uint8> output, Int32 width, Int32 height);
typedef NativeDart = int Function(
  Pointer<Uint8> input, Pointer<Uint8> output, int width, int height);

class NativeProcess {
  final DynamicLibrary dylib;

  NativeProcess(String libName): dylib = DynamicLibrary.open(libName);

  void run(String functionName,
    Pointer<Uint8> input, Pointer<Uint8> output, int width, int height) {
    final NativeDart nativeEntry =
      dylib.lookupFunction<NativeC, NativeDart>(functionName);

    final result = nativeEntry(input, output, width, height);
    if(result != 0){
      throw Exception("Err Function");
    }
  }
}