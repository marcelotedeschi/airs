/*
 *  AIRS: Automated Irrigação System
 *  Version: 1.0
 *  Created by: Marcelo Tedeschi
 *  License: MIT
 *  Date: 18/NOV/2017
 */

#include <TimeLib.h>

// Declare number of sensors
const int NUMBER_OF_SENSORS = 6;

// Declare Analog inputs being used for humidity sensors
const int H_SENSORS[NUMBER_OF_SENSORS] = {A0, A1, A2, A3, A4, A5};
int humidity;

// Declare relay port
const int RELAY_PORT = 13;

// Declare set time ports
const int CLOCK_BUTTON = 10;
const int CLOCK_SET_LED = 11;
const int GREEN_LED = 12;
const int NIGHT_TIME = 19;

// Declare default irrigation time in minutes
const int IRRIGATION_TIME = 5;

// Declare initial board time
int current_hour = hour();

// Declare humidity limits (the lower the more humid)
// Values from https://www.filipeflop.com/blog/monitore-sua-planta-usando-arduino/
const int HUMIDITY_LOW_LIMIT = 800;
const int HUMIDITY_HIGH_LIMIT = 400;

// Initial Board Setup
void setup() {
  // Setup pins
  pinMode(RELAY_PORT, OUTPUT);
  pinMode(CLOCK_BUTTON, INPUT);
  pinMode(CLOCK_SET_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  for (int i = 0; i < NUMBER_OF_SENSORS; i++) {
    pinMode(H_SENSORS[i], INPUT);
  }

  Serial.begin(9600);
}

// Main loop function
void loop() {
  // Only check humidity once every hour
  if (canGetHumidity(current_hour)) {
    Serial.println("Getting humidity...");
    humidity = getHumidity(H_SENSORS);

    if (isHumidityLow(humidity) && isNightTime(current_hour)) {
      Serial.println("Irrigating...");
      // run irrigation system
      activateIrrigation();
    }
  }

  if (digitalRead(CLOCK_BUTTON) == HIGH) {
    Serial.println("Setting time...");
    setBoardTime();
  }

  delay(500);
  Serial.print("system hour: ");
  Serial.println(hour());
  Serial.print("current hour: ");
  Serial.println(current_hour);
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

  if (humidity < HUMIDITY_HIGH_LIMIT) {
    return true;
  }

  return false;
}

bool isHumidityLow(int humidity) {
  if (humidity > HUMIDITY_LOW_LIMIT) {
    Serial.println("Humidity Low");
    return true;
  }

  return false;
}

bool isNightTime(int hour) {
  if (hour > NIGHT_TIME) {
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

    if (type == 'minute' || type == 'second') {
      t1 += 60;
    }
  }

  return t1 - t2;
}

// Blinks led by entering led port, delay time between blinks and blink times
void blinkLed(int port, int d, int times = 1) {
  for (int i = 0; i < times; i++) {
    digitalWrite(port, HIGH);
    delay(d);
    digitalWrite(port, LOW);

    if (times > 1) {
      delay(d);
    }
  }
}

/* Sets time using a button
 * 1. Click button once (green light turns on, set is enabled)
 * 2. Click button the amount of hours you want to set, waiting for
 *    red led to turn off between clicks
 * 3. Wait 5 seconds, and it sets the time
 */
void setBoardTime() {
  int hour_count = 0;
  bool buttonPressed = false;
  time_t t_temp = now();

  Serial.print("Current hour: ");
  Serial.println(current_hour);

  digitalWrite(GREEN_LED, HIGH);
  delay(2000);

  while (buttonPressed || timeDiff(second(), t_temp, 'second') < 5) {
    buttonPressed = false;

    if (digitalRead(CLOCK_BUTTON) == HIGH) {
      hour_count++;
      t_temp = now();

      if (hour_count == 24) {
        hour_count = 0;
      }

      buttonPressed = true;
      digitalWrite(CLOCK_SET_LED, HIGH);
      delay(800);
      digitalWrite(CLOCK_SET_LED, LOW);
    }
  }

  current_hour = hour_count;
  setTime(hour_count, 0, 0, 19, 11, 17);

  Serial.print("Hour set to ");
  Serial.println(hour_count);

  // Blink green light to signal success
  digitalWrite(GREEN_LED, LOW);
  delay(100);
  blinkLed(GREEN_LED, 100, 2);
  blinkLed(CLOCK_SET_LED, 500, hour_count);
}
