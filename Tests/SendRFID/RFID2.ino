/*
 * 
 * All the resources for this project: https://www.hackster.io/Aritro
 * Modified by Aritro Mukherjee
 * 
 * 
 */
 
#include <SPI.h>
#include <MFRC522.h>
 
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

int bit1    = 4;
int bit2    = 5;
int readOff = 6;
int readOn  = 7;

bool RFIDOff = false;

int state = 0;

int stateOfOFFPin;
int stateOfOnPin;
 
void setup() {
  
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  pinMode(bit1, OUTPUT);
  pinMode(bit2, OUTPUT);
  
  pinMode(readOff, INPUT);
  pinMode(readOn,  INPUT);


}

void loop() {

  //RFID ON
  if(state == 0) { 
    digitalWrite(bit1, LOW);
    digitalWrite(bit2, LOW);
    setPinOUT();
  }

  /**
   * Send ACCEPT output (USER 1)
   * 
   *   0  0
   */
  if(state == 1) {
    digitalWrite(bit1, LOW);
    digitalWrite(bit2, HIGH);
    checkRFIDOff();
  }


  /**
   * Send DENY output 
   * 
   *   1  1
   */ 
  if(state == 3) {
    digitalWrite(bit1, HIGH);
    digitalWrite(bit2, HIGH);
    checkRFIDOff();
  }



  //RFID OFF
  if(state == 4) {
    Serial.println("Achieved State 4");
    checkRFIDOn();
  }

  Serial.println(state);
  

} 


void setPinOUT() {

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {   
    return; 
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) { 
    return; 
    }

  //Show UID on serial monitor
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  content.toUpperCase();
  if (content.substring(1) == "DA 4B 97 1A") {
    state = 1;
  }
 
 else   {
    state = 3;
  }
  
}


/**
 * When drone is accepted/declined, it sends a message to main RFID
 * Main RFID will send a confirmation HIGH back, indicating RFID Off
 * This turns RFID Off
 * ON = 1
 * OFF = 2
 */
void checkRFIDOff() {
  
  stateOfOFFPin = digitalRead(readOff);

  Serial.print("Here is the state of the OFF pin: ");
  Serial.println(stateOfOFFPin);
  
  if(stateOfOFFPin == 1) {
    Serial.println("We have an RFID HIGH");
    state = 4;
  }
  
}

void checkRFIDOn() {
  
  stateOfOnPin = digitalRead(readOn);

  Serial.print("Here is the state of the ON pin: ");
  Serial.println(stateOfOnPin);

  if(stateOfOnPin == 1) {
    state = 0;
  }
}
