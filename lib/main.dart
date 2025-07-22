import 'package:cvapp/bridge/bridge.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:image/image.dart' as img;

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      home: ImageComparisonPage(),
    );
  }
}

class ImageComparisonPage extends StatefulWidget {
  const ImageComparisonPage({super.key});

  @override
  State<ImageComparisonPage> createState() => _ImageComparisonPageState();
}

img.Image _convertGray(Map<String, dynamic> params) {
  img.Image image = params["image"];
  return Bridge.lumDoc(image);
  // return NativeControl.convertGray(image: image);
}

class _ImageComparisonPageState extends State<ImageComparisonPage> {
  Uint8List? _imageBytes;

  Future<void> _toggleImage() async {
    ByteData data = await rootBundle.load("assets/frame.jpg");
    Uint8List bytes = data.buffer.asUint8List();
    img.Image? image = img.decodeImage(bytes);
    image!;

    img.Image grayImage = await compute(_convertGray, {
      "image": image,
    });

    setState(() {
      _imageBytes = Uint8List.fromList(img.encodePng(grayImage));
    });
  }

  Future<void> _loadImage() async {
    ByteData data = await rootBundle.load('assets/frame.jpg');
    Uint8List bytes = data.buffer.asUint8List();

    // img.Image -> Uint8List
    img.Image? image = img.decodeImage(bytes);
    if (image != null) {
      setState(() {
        _imageBytes = Uint8List.fromList(img.encodePng(image));
      });
    }
  }

  @override
  void initState() {
    super.initState();
    _loadImage();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Before & After Comparison')),
      body: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          _imageBytes != null
              ? Image.memory(_imageBytes!)
              : CircularProgressIndicator(),
          SizedBox(height: 20),
          ElevatedButton(
            onPressed: _toggleImage,
            child: Text('convert'),
          ),
        ],
      ),
    );
  }
}
