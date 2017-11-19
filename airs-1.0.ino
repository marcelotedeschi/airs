/*
 *  AIRS: Automated Irrigação System
 *  Version: 1.0
 *  Created by: Marcelo Tedeschi
 *  License: MIT
 *  Date: 18/NOV/2017
 */

#include <TimeLib.h>

// Declare number of sensors
const int NUMBER_OF_SENSORS = 3;

// Declare Analog inputs being used for humidity sensors
const int H_SENSORS[NUMBER_OF_SENSORS] = {A0, A1, A2};
int humidity;

// Declare relay port
const int RELAY_PORT = 13;

// Declare default irrigation time in minutes
const int IRRIGATION_TIME = 5;

// Declare light sensor port
const int LIGHT_SENSOR_PORT = A5;
const int LIGHT_SENSOR_LIMIT = 500;

// Declare initial board time
time_t current_hour = now();

// Declare low humidity limit
const int HUMIDITY_LOW_LIMIT = 100; // TODO find correct humidity value
const int HUMIDITY_HIGH_LIMIT = 1000;

// Initial Board Setup
void setup() {
  // Setup pins
  pinMode(RELAY_PORT, OUTPUT);
  Serial.begin(9600);
}

// Main loop function
void loop() {
  // Only check humidity once every hour
  if (canGetHumidity(current_hour)) {
    Serial.println("Getting humidity...");
    humidity = getHumidity(H_SENSORS);

    if (isHumidityLow(humidity) && isDarkEnough(LIGHT_SENSOR_PORT)) {
      Serial.println("Irrigating...");
      // run irrigation system
      activateIrrigation();
    }
  }

  delay(500);
}

void activateIrrigation() {
  time_t t = now();
  int timeDifference = 0;

  // Activate Relay
  digitalWrite(RELAY_PORT, HIGH);

  // Loop for 5 minutes
  while (isEnoughHumidity() == false || timeDifference < IRRIGATION_TIME) {
    timeDifference = timeDiff(minute(), t, 'minute');
    Serial.print(5 - timeDifference);
    Serial.println(" minutes remaining");
    delay(2000);
  }

  // Deactivate Relay
  digitalWrite(RELAY_PORT, LOW);
}

// Calculate humidity average value
int getHumidity(int h_sensors[]) {
  int humidity, sensor_value;
  // TODO activate/deactivate sensors with digital ports

  for (int i=0; i < NUMBER_OF_SENSORS; i++) {
    sensor_value = analogRead(h_sensors[i]);
    humidity += sensor_value;
  }

  humidity = humidity / NUMBER_OF_SENSORS;
  return humidity;
}

// Returns true every hour and updates current_hour
bool canGetHumidity(time_t current_hour) {
  time_t t = hour(); // get hour

  if (timeDiff(t, current_hour, 'hour') > 0) {
    current_hour = t; // updates current_time to the current hour
    return true;
  }

  return false;
}

// Returns true if humidity reaches high limit
bool isEnoughHumidity() {
  int humidity = getHumidity(H_SENSORS);

  if (humidity > HUMIDITY_HIGH_LIMIT) {
    return true;
  }

  return false;
}

bool isHumidityLow(int humidity) {
  if (humidity < HUMIDITY_LOW_LIMIT) {
    Serial.println("Humidity Low");
    return true;
  }

  return false;
}

bool isDarkEnough(int light_sensor) {
  if (analogRead(light_sensor) < LIGHT_SENSOR_LIMIT) {
    Serial.println("Night time");
    return true;
  }

  return false;
}

// Util functions

// Calculates time difference (higher - lower) in 24 hour, 60 minute clock
int timeDiff(int t1, int t2, char type) {
  if (t1 < t2) {
    if (type == 'hour') {
      t1 += 24;
    }

    if (type == 'minute') {
      t1 += 60;
    }
  }

  return t1 - t2;
}
