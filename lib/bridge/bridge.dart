import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:image/image.dart';
import 'package:cvapp/bridge/native_process.dart';

class Bridge {
  static void image2buffer({required Image image,
    required Pointer<Uint8> buffer}){
    final width = image.width;
    final height = image.height;

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        // row first
        Pixel pixel = image.getPixel(x, y);

        final base = (y * width + x) * 3;
        buffer[base + 0] = pixel.r.toInt();
        buffer[base + 1] = pixel.g.toInt();
        buffer[base + 2] = pixel.b.toInt();
      }
    }
  }
  static void buffer2image({required Pointer<Uint8> buffer,
    required Image image
  }){
    final width = image.width;
    final height = image.height;

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        // row first
        final base = (y * width + x) * 3;
        int r = buffer[base + 0];
        int g = buffer[base + 1];
        int b = buffer[base + 2];

        image.setPixel(x, y, ColorInt8.rgb(r, g, b));
      }
    }
  }

  static Image lumDoc(Image image){
    final width = image.width;
    final height = image.height;

    Pointer<Uint8> bufferIn = calloc<Uint8>(3 * width * height);
    Pointer<Uint8> bufferOut = calloc<Uint8>(3 * width * height);

    try{
      image2buffer(image: image, buffer: bufferIn);
      NativeProcess("libdocscanner.so").run('lum_doc',
        bufferIn, bufferOut, width, height);

      var result = Image(width: width, height: height);
      buffer2image(buffer: bufferOut, image: result);

      return result;
    } finally{
      calloc.free(bufferIn);
      calloc.free(bufferOut);
    }
  }
}