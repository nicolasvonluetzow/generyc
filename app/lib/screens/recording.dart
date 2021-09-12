// shows the live readings

import 'dart:async';
import 'dart:math';

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';

class Recording extends StatefulWidget {
  const Recording({
    Key? key,
  }) : super(key: key);

  @override
  State<Recording> createState() => _RecordingState();
}

class _RecordingState extends State<Recording> {
  List<double> _results = [];
  bool recording = false;
  Timer? timer;

  BarChartData getChartData() {
    return BarChartData(
      titlesData: FlTitlesData(
        rightTitles: SideTitles(showTitles: false),
        leftTitles: SideTitles(showTitles: false),
        topTitles: SideTitles(showTitles: false),
      ),
      borderData: FlBorderData(
        show: false,
      ),
      barGroups: getBarGroups(),
      gridData: FlGridData(show: false),
    );
  }

  List<BarChartGroupData> getBarGroups() {
    List<BarChartGroupData> barGroups = [];
    for (int i = 0; i < _results.length; i++) {
      double result = _results[i];
      barGroups.add(BarChartGroupData(
          x: i + 1, barRods: [BarChartRodData(y: result, width: 10)]));
    }
    return barGroups;
  }

  List<DataRow> getResultsWidgets() {
    List<DataRow> rows = [];
    for (int i = 0; i < _results.length; i++) {
      int index = _results.length - i - 1;
      double result = _results[index];
      rows.add(
        DataRow(
          cells: [
            DataCell(
              Text(
                (index + 1).toString(),
              ),
            ),
            DataCell(
              Text(
                result.toStringAsFixed(2),
              ),
            ),
          ],
        ),
      );
    }
    return rows;
  }

  String dropdownVal = "Pull-Up";
  final List<DropdownMenuItem<String>> supportedExercises = ["Pull-Up", "Push-Up", "Squat"].map<DropdownMenuItem<String>>((String value) {
  return DropdownMenuItem<String>(child: Text(value), value: value);
  }).toList();

  @override
  Widget build(BuildContext context) {
    print(supportedExercises);
    return Scaffold(
      appBar: AppBar(
        title: DropdownButtonHideUnderline(
          child: DropdownButton(
            value: dropdownVal,
            icon: Icon(
              Icons.arrow_drop_down_rounded,
            ),
            style: TextStyle(color: Colors.black, fontSize: 20),
            onChanged: (String? newVal) {
              setState(() {
                dropdownVal = newVal!;
                _results = [];
              });
            },
            items: supportedExercises,
          ),
        ),
        elevation: 0,
        backgroundColor: Colors.transparent,
        bottomOpacity: 0,
      ),
      body: Padding(
        padding: const EdgeInsets.all(8.0),
        child: _results.length == 0
            ? Center(
                child: Text(
                "Tap on the record button to start measuring the exercise velocity!",
                textAlign: TextAlign.center,
                textScaleFactor: 1.8,
              ))
            : Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  Flexible(
                    child: Padding(
                      padding: const EdgeInsets.symmetric(vertical: 20),
                      child: BarChart(
                        getChartData(),
                      ),
                    ),
                    flex: 2,
                  ),
                  Flexible(
                      child: Padding(
                    padding: const EdgeInsets.symmetric(
                        vertical: 10, horizontal: 30),
                    child: SingleChildScrollView(
                      child: DataTable(
                        columns: const <DataColumn>[
                          DataColumn(
                            label: Text(
                              'Rep',
                              style: TextStyle(fontStyle: FontStyle.italic),
                            ),
                          ),
                          DataColumn(
                            label: Text(
                              'Velocity (m/s)',
                              style: TextStyle(fontStyle: FontStyle.italic),
                            ),
                          ),
                        ],
                        rows: getResultsWidgets(),
                      ),
                    ),
                  )),
                ],
              ),
      ),
      floatingActionButton: FloatingActionButton(
        child: Icon(recording ? Icons.stop : Icons.play_arrow_rounded),
        onPressed: () {
          if (recording) {
            // stopping the recording
            timer?.cancel();
          } else {
            // starting the recording
            // todo subscribe to the ble stream, and replace the timer with the subscription
            timer = Timer.periodic(
              Duration(seconds: 2),
              (Timer t) => setState(
                () {
                  _results.add(1 + Random().nextDouble());
                },
              ),
            );
          }
          setState(() {
            recording = !recording;
          });
        },
      ),
    );
  }
}
