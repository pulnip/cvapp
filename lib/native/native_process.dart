import 'dart:ffi';

typedef NativeEntryC = Int32 Function(
  Pointer<Uint8> input, Pointer<Uint8> output, Int32 width, Int32 height);
typedef NativeEntryDart = int Function(
  Pointer<Uint8> input, Pointer<Uint8> output, int width, int height);

class NativeProcess {
  static final DynamicLibrary _dylib = DynamicLibrary.open('libdocscanner.so');

  static void nativeEntry(Pointer<Uint8> input,
    Pointer<Uint8> output, int width, int height) {
    final NativeEntryDart nativeEntry =
      _dylib.lookupFunction<NativeEntryC, NativeEntryDart>(
        'native_entry');

    final result = nativeEntry(input, output, width, height);
    if(result != 0){
      throw Exception("Err native_entry");
    }
  }
}