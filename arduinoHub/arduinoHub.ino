#include <IRremote.h>

//INPUT PINS
int tempSensor = A1;
int remotePin = 3;
int soundSensor = 5;
//OUTPUT PINS
int purple_relay = 8;
int green_relay = 10;
int led_relay = 13;

//CONSTANTS
const int tempCatch = 5000;

//Global States
int clapsCounter = 0;
boolean lightState = true;
bool remoteLedState = false;
float temp = 0.0;

boolean clap_changed = false;
boolean remote_changed = false;

//Remote
IRrecv irrecv(remotePin);
decode_results remoteState;

//timespans
unsigned long tempSpan = millis();
unsigned long clapDetectionSpanInitial = millis();
unsigned long clapDetectionSpan = 0;
unsigned long remoteSignalLastCheck = millis();

void setup() {
  Serial.begin(9600);
  pinMode(soundSensor, INPUT);
  pinMode(purple_relay, OUTPUT);
  pinMode(green_relay, OUTPUT);
  pinMode(led_relay, OUTPUT);

  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  //initial reads
  int sensorState = digitalRead(soundSensor);
  float reading = analogRead(tempSensor);

  //remote read
  if (irrecv.decode(&remoteState)) {
    // If it's been at least 1/4 second since the last
    // IR received, toggle the relay
    if (millis() - remoteSignalLastCheck > 250) {
      remote_changed = true;
      if (remote_led_on(&remoteState)) {
        remoteLedState = !remoteLedState;
      }
    }
    remoteSignalLastCheck = millis();
    irrecv.resume(); // Receive the next value
  }

  //read temperature
  if (millis() - tempSpan > tempCatch) {
    temp = get_temperature(reading);
    tempSpan = millis();
  }

  //update spans
  //tempSpan = tempSpan + ( millis() tempSpan

  //execute logic
  boolean clapStatus = clap_sensor(sensorState, led_relay, clap_changed);

  //Actualize relays
  if (clap_changed || remote_changed) {
    actualize_pin(led_relay, remoteLedState);
    actualize_pin(purple_relay, remoteLedState);
    actualize_pin(purple_relay, remoteLedState);
  }

  //clean notifications
  clap_changed = false;
  remote_changed = false;

  //logging
  //Serial.print(sensorState);
  //Serial.print(" - ");
}

boolean clap_sensor(int clapSensorStatus, int relay, boolean relay_changed_notification) {
  int totalMillisForChangeStatus = 700;
  int totalMillisBetweenClaps = 100;
  if (clapSensorStatus == 0)
  {
    if (clapsCounter == 0)
    {
      clapDetectionSpanInitial = clapDetectionSpan = millis();
      clapsCounter++;
    }
    else if (clapsCounter > 0 && millis() - clapDetectionSpan >= totalMillisBetweenClaps)
    {
      clapDetectionSpan = millis();
      clapsCounter++;
    }
  }

  if (millis() - clapDetectionSpanInitial >= totalMillisForChangeStatus)
  {
    if (clapsCounter == 2)
    {
      lightState = !lightState;
      relay_changed_notification = true;
    }
    clapsCounter = 0;
  }
  return lightState;
}

boolean read_save_temp() {
  temp = analogRead(tempSensor);
  Serial.print("Temp is ");
  Serial.println(temp);
  return temp;
}

void actualize_pin(int relay, boolean high) {
  if (high) {
    digitalWrite(relay, HIGH);
  }
  else {
    digitalWrite(relay, LOW);
  }
}

boolean remote_led_on(decode_results *results) {
  String hex_value = dump_remote_signal(results);
  return hex_value == "ff9867";
}

String dump_remote_signal(decode_results *results) {
  String hex_value = String(results->value, HEX);
  Serial.print(hex_value);
  return hex_value;
}

float get_temperature(float val) {
  float mv = ( val / 1024.0) * 5000;
  float cel = mv / 10;
  Serial.print(cel); Serial.println(" degrees C");

  return cel;
}

