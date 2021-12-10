import 'dart:typed_data';

import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:sensors_plus/sensors_plus.dart';
import 'package:flutter/services.dart';
import 'package:usb_serial/usb_serial.dart';
import 'dart:math';
import 'dart:core';
import 'package:google_fonts/google_fonts.dart';
import 'Point.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    // Set landscape orientation
    SystemChrome.setPreferredOrientations([
      DeviceOrientation.landscapeLeft,
      DeviceOrientation.landscapeRight,
    ]);
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.green,
        //GoogleFonts.ubuntu()
        textTheme: GoogleFonts.ubuntuMonoTextTheme(
          Theme.of(context).textTheme,
        ),
      ),
      home: MyHomePage(title: 'MagVibe Application'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  MyHomePage({Key key, this.title}) : super(key: key);

  final String title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  var xVal = 0.0;
  var yVal = 0.0;
  var zVal = 0.0;
  List<double> dummy = [0.0, 0.0, 0.0];
  List<double> currentCursor = [0.0, 0.0, 0.0];
  List<String> keys = ["1", "2", "3", "4", "5", "6", "C", "0", "E"];
  List<List<int>> distancesToCenter = [
    [5, 3, 5],
    [4, 0, 4],
    [5, 3, 5]
  ];
  String msg = "";
  var currentPointIndex;
  var minDistance = 3.5;
  var minDistances = [5.5, 4.5, 2];
  var debounceTime = 400;
  var distCur = 0.0;
  var rows = 3;
  var cols = 3;
  var listSize = 0;
  var points;
  var isInRange = false;
  var previousRange = true;
  var useVectorDistance = true;
  Stopwatch stopwatch;

  UsbPort port;

  double map(
      double x, double inMin, double inMax, double outMin, double outMax) {
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
  }

  double distance(List<double> a, List<double> b) {
    return sqrt(
        pow((b[0] - a[0]), 2) + pow((b[1] - a[1]), 2) + pow((b[2] - a[2]), 2));
  }

  double distance2(List<double> a, List<double> b) {
    return (b[0] - a[0]).abs() + (b[1] - a[1]).abs() + (b[2] - a[2]).abs();
  }

  Future<void> initSerial() async {
    List<UsbDevice> devices = await UsbSerial.listDevices();
    print(devices);

    //UsbPort port;
    if (devices.length == 0) {
      return;
    }
    port = await devices[0].create();

    bool openResult = await port.open();
    if (!openResult) {
      print("Failed to open");
      return;
    }

    await port.setDTR(true);
    await port.setRTS(true);

    port.setPortParameters(
        9600, UsbPort.DATABITS_8, UsbPort.STOPBITS_1, UsbPort.PARITY_NONE);

    // print first result and close port.
    port.inputStream.listen((Uint8List event) {
      print(event);

      //port.close();
    });
  }

  Future<void> click(Point p) async {
    String wordToSend = "click" + p.distanceToCenter.toString() + "\n";
    await port.write(Uint8List.fromList(wordToSend.codeUnits));
  }

  Future<void> ledOn() async {
    await port.write(Uint8List.fromList("on\n".codeUnits));
  }

  Future<void> ledOff() async {
    await port.write(Uint8List.fromList("off\n".codeUnits));
  }

  @override
  void initState() {
    // TODO: implement initState
    super.initState();
    //initSerial();
    stopwatch = new Stopwatch();
    listSize = rows * cols;
    points = List<Point>.filled(listSize, null);
    int index = 0;
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        points[index] = new Point(dummy, 0.0, map(c.toDouble(), 0, 2, -1, 1),
            map(r.toDouble(), 0, 2, -1, 1), distancesToCenter[r][c]);
        index++;
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    Future<MagnetometerEvent> e = magnetometerEvents.first;
    e.then((value) => {
          setState(() {
            xVal = value.x;
            yVal = value.y;
            zVal = value.z;
            currentCursor[0] = xVal;
            currentCursor[1] = yVal;
            currentCursor[2] = zVal;
            //distCur = distance(tL, currentCursor);
          })
        });

    double min = 1000.0;
    var alignX = 0.0;
    var alignY = 0.0;
    for (int i = 0; i < listSize; i++) {
      points[i].distanceToCursor = useVectorDistance
          ? distance(currentCursor, points[i].position)
          : distance2(currentCursor, points[i].position);
      if (points[i].distanceToCursor < min) {
        currentPointIndex = i;
        min = points[i].distanceToCursor;
        if (min < minDistances[(points[i].y).toInt()+1]) {
          if (!isInRange) {
            isInRange = true;
            stopwatch..start();
          }
        } else {
          isInRange = false;
        }
        alignX = points[i].x;
        alignY = points[i].y;
      }
    }

    if (isInRange) {
      print(currentPointIndex);
      if (stopwatch.elapsedMilliseconds > debounceTime) {
        stopwatch.reset();
        setState(() {
          msg += keys[currentPointIndex];
        });
        //Add Arduino Click here...
        click(points[currentPointIndex]);
        //-------------------------
      }
    } else {
      stopwatch.reset();
    }

    return Scaffold(
        appBar: AppBar(
          title: Text(widget.title),
          actions: [
            IconButton(
                icon: useVectorDistance ? Icon(Icons.adjust) : Icon(Icons.height),
                onPressed: () {
                  setState(() {
                    useVectorDistance = !useVectorDistance;
                  });
                }),
            IconButton(icon: Icon(Icons.usb), onPressed: initSerial),
            IconButton(
              icon: Icon(Icons.lightbulb),
              onPressed: ledOn,
            ),
            IconButton(
              icon: Icon(Icons.lightbulb_outline),
              onPressed: ledOff,
            ),
          ],
        ),
        body: Row(
          children: [
            Column(
              children: [
                Text("X: " + xVal.toStringAsPrecision(5).toString(),
                    style: Theme.of(context).textTheme.bodyText1),
                Text("Y: " + yVal.toStringAsPrecision(5).toString(),
                    style: Theme.of(context).textTheme.bodyText1),
                Text("Z: " + zVal.toStringAsPrecision(5).toString(),
                    style: Theme.of(context).textTheme.bodyText1),
                Text("Min. Dist: " + minDistance.toString()),
                Text("Time: " + debounceTime.toString() + "ms"),
                Container(
                  width: 120,
                  child: Text(
                    msg,
                    style: TextStyle(fontSize: 16),
                  ),
                ),
                OutlinedButton(
                  onPressed: () {
                    setState(() {
                      msg = "";
                    });
                  },
                  child: Text("Reset"),
                )
              ],
            ),
            Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: <Widget>[
                  Padding(
                    padding: const EdgeInsets.fromLTRB(40, 0, 0, 0),
                    child: Container(
                        height: 230,
                        width: 250,
                        decoration: BoxDecoration(
                            border:
                                Border.all(color: Colors.green, width: 3.0)),
                        alignment: Alignment(alignX, alignY),
                        child: isInRange
                            ? Material(
                                //elevation: map(zVal, tL[2], bR[2], 0, 20) >= 0 ? map(zVal, tL[2], bR[2], 0, 20) : 0,
                                child: Container(
                                  color: Colors.green,
                                  height: 10,
                                  width: 10,
                                ),
                              )
                            : Text("")),
                  ),
                ],
              ),
            ),
            Container(
              height: 280,
              width: 300,
              child: GridView.builder(
                gridDelegate: SliverGridDelegateWithFixedCrossAxisCount(
                  crossAxisCount: cols,
                ),
                itemCount: listSize,
                itemBuilder: (BuildContext context, int index) {
                  return Column(
                    children: [
                      OutlinedButton(
                          onPressed: () {
                            List<double> pos = [xVal, yVal, zVal];
                            points[index].position = pos;
                          },
                          child: Text(keys[index])),
                      Text(index.toString() +
                          " - " +
                          points[index]
                              .distanceToCursor
                              .toStringAsPrecision(5)
                              .toString()),
                    ],
                  );
                },
              ),
            ),
          ],
        ) // This trailing comma makes auto-formatting nicer for build methods.
        );
  }
}
