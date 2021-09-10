/*
   First we rotate the linear acceleration into earths frame of reference.
   Then we calculate the rolling average of the last measurements.
   TODO: We should add calibration of the axis to minimize integration drift (e.g. sitting the sensor on a table for 10s)
   Then we calculate horizontal (y,z) and vertical (x) components.
   TODO: Afterwards we pattern match in the horizontal and vertcal data

   threshold: 500
   determine crossing point and local max/min of last 3s
   if n points of last 3s are bigger than threshold and axis was crossed:
      determine min/max


   Deps:
   - https://github.com/JChristensen/movingAvg
   - https://github.com/RCmags/vector_datatype
*/

#include "Arduino.h"
#include "Arduino_BHY2.h"
#include "quaternion_type.h"
#include "vector_type.h"
#include "movingAvg.h"

#define RUNNING_AVG_LEN 128
#define SAMPLE_RATE 800
#define approximated_velocity_SECONDS 1
#define approximated_velocity_LEN 200
#define TRIGGER_THRESHOLD 300
#define TIME_CROSS_THRESHOLD 75
#define BUFFER_SECONDS 3
#define TIME_REP_THRESHOLD (BUFFER_SECONDS * 1000)
#define approximated_velocity_THRESHOLD 300
#define BUFFER_SIZE (SAMPLE_RATE * BUFFER_SECONDS)

SensorXYZ linAccel(SENSOR_ID_LACC);
SensorQuaternion orientation(SENSOR_ID_RV);

/*movingAvg x_avg(RUNNING_AVG_LEN);
movingAvg y_avg(RUNNING_AVG_LEN);*/
movingAvg z_avg(RUNNING_AVG_LEN);

int all_readings[50];
int all_readings_index = 0;

int sum_since_last_cross = 0, readings_since_last_cross = 0;


unsigned long printTime = millis();
unsigned long lastCross = 0;
int lastCrossIndex = 0, xAbsapproximated_velocitySum = 0, matchCounter = 0;
bool lastCrossWasIntoPositive = true;

void printAvg( double x_sum, double y_sum, double z_sum) {
  Serial.print( "x: " );
  //Serial.print( x_sum );
  //Serial.print( ", y: " );
  Serial.print( y_sum );
  Serial.print( ", z: " );
  Serial.print( z_sum );
  /*Serial.print( ", no: " );
  Serial.print( 0 );
  /*Serial.print( ", sum: " );
    Serial.print( sqrt(sq(z_sum)+sq(y_sum)+sq(x_sum) ));*/
  Serial.println();
}


void setup()
{
  Serial.begin(115200);
  while (!Serial);

  BHY2.begin();

  /*x_avg.begin();
  y_avg.begin();*/
  z_avg.begin();

  linAccel.configure(SAMPLE_RATE, 0);
  orientation.configure(SAMPLE_RATE, 0);
}

void loop()
{
  unsigned long currentMillis = millis();

  // Update function should be continuously polled
  BHY2.update();

  if (currentMillis - printTime >= 1 / SAMPLE_RATE) {
    printTime = currentMillis;

    vec3_t rotated_accel = getRotatedAccel();
    /*int x_prev_avg = x_avg.getAvg(), x_next_avg = x_avg.reading(rotated_accel.x);
    int y_prev_avg = y_avg.getAvg(), y_next_avg = y_avg.reading(rotated_accel.y);*/
    int z_prev_avg = z_avg.getAvg(), z_next_avg = z_avg.reading(rotated_accel.z);
    all_readings[all_readings_index] = z_next_avg;
    
    all_readings_index = (all_readings_index + 1) % BUFFER_SIZE;
    //if (all_readings_index < BUFFER_SIZE-1) all_readings_index++;

    sum_since_last_cross += abs(z_next_avg); 
    readings_since_last_cross++;

    bool hasCrossed = crossesZero(z_prev_avg, z_next_avg), fellBelowThreshold = fallsBelowThreshold(z_prev_avg, z_next_avg);
    bool lastCrossWasLongTimeAgo = (currentMillis - lastCross > TIME_CROSS_THRESHOLD);
    bool lastRepWasLongTimeAgo = currentMillis - lastCross > TIME_REP_THRESHOLD;

    if (lastRepWasLongTimeAgo) {
      sum_since_last_cross = 0;
      readings_since_last_cross = 0;
    }

    
    double approximated_velocity = abs((sum_since_last_cross / readings_since_last_cross) * (currentMillis - lastCross));
    bool absAvgAboveThreshold = approximated_velocity > approximated_velocity_THRESHOLD;

    if (hasCrossed && absAvgAboveThreshold && lastCrossWasLongTimeAgo ) {
      //if (); // if the last cross was long time ago => new rep
      matchCounter++;
      printAvg(0, approximated_velocity / 10000, z_next_avg);

    } else {

      printAvg(0, approximated_velocity / 10000, z_next_avg);
    }



    if (hasCrossed) {
      lastCross = currentMillis;
      lastCrossIndex = lastCrossIndex;
      lastCrossWasIntoPositive = z_next_avg >= 0;

      sum_since_last_cross = 0;
      readings_since_last_cross = 0;
    }

  }
}


vec3_t getRotatedAccel() {
  vec3_t accel = { linAccel.x() * 2, linAccel.y() * 2, linAccel.z() * 2 };
  quat_t quat = { orientation.w(), orientation.x(), orientation.y(), orientation.z() };
  quat = (quat * pow(2, -14));
  quat = quat.norm();
  vec3_t rotated_accel = (quat * accel * quat.conj()).v * 2; // is the * 2 necessary?
  return rotated_accel;
}


bool crossesZero(int prev, int next) {
  // returns true if the zero point was crossed
  bool prev_sign = prev > 0;
  bool next_sign = next > 0;
  return prev_sign != next_sign;
}

bool fallsBelowThreshold(int prev, int next) {
  return prev >= TRIGGER_THRESHOLD && next < TRIGGER_THRESHOLD;
}
