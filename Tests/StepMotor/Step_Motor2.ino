/*
 Stepper Motor Control - one revolution

 This program drives a unipolar or bipolar stepper motor.
 The motor is attached to digital pins 8 - 11 of the Arduino.

 The motor should revolve one revolution in one direction, then
 one revolution in the other direction.


 Created 11 Mar. 2007
 Modified 30 Nov. 2009
 by Tom Igoe

 */


//State of the antenna (changes with button)
int antennaState = LOW;
int counter = 0;
int previousState = LOW;

const int buttonPin = 2;

void setup() {
  pinMode(buttonPin, INPUT);
  Serial.begin(9600);
}

void loop() {

  //Antenna State begins as LOW
  
  antennaState = digitalRead(buttonPin);

  if(antennaState != previousState) {
    if(antennaState == HIGH) {
      counter ++;
      Serial.println("UP");
    } else {
      Serial.println("DOWN");
    }
    delay(50);
  }
  previousState = antennaState;

  



}

  
  



  
