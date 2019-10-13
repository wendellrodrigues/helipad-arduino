#include <Ethernet.h>
#include <SPI.h>

//Variables for server connection
byte mac[]      = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   //Self-Assigned MAC address
byte ip[]       = { 10, 0, 0, 177 };                        //Self-Assigned IP for arduino
byte server[]   = { 10, 0, 0, 248 };                        //IP of laptop (where server is locally hosted)

//For sending data to server
EthernetClient client;

//Declare two variables
float cm;
float ECHOTIME;

//Declare a count
int ultraSonicCount = 0;

//Landed variables
bool    landed       = false;
double  timeToLand   = 0; //Time it takes to land
double  parkedTime   = 0; //Time the drone has been parked in the spot


//Button Pins (
int buttonAccept  = 48;
int buttonDeny    = 50;
int takeOff       = 52;

//Initialize Trigger and Echo Pins
const int TrigPin = 2;
const int EchoPin = 3;

int LEDzeroState  = 22;
int LEDaccept     = 24;
int LEDdeny       = 26;
int LEDtakeOff    = 28;

int bit1      = 30;   //RFID 4  -> MAIN 30
int bit2      = 31;   //RFID 5  -> MAIN 31
int sendOff   = 32;   //MAIN 32 -> RFID 6
int sendOn    = 33;   //MAIN 33 -> RFID 7




//Needed to track state (resets to zero accordingly)
int buttonAcceptCounter   = 0;
int buttonDenyCounter     = 0;
int buttonTakeOffCounter  = 0;

//Overall State
int STATE = 0;


void setup() {

  //Begin output
  Serial.begin(9600);

  //Pins for RFID
  pinMode(bit1,                         INPUT);
  pinMode(bit2,                         INPUT);
  pinMode(sendOff,                      OUTPUT);
  pinMode(sendOn,                       OUTPUT);

  //Pins for the buttons
  pinMode(buttonAccept, INPUT);
  pinMode(buttonDeny,   INPUT);
  pinMode(takeOff,      INPUT);

  //Pins for the LEDs
  pinMode(LEDzeroState, OUTPUT);
  pinMode(LEDaccept,    OUTPUT);
  pinMode(LEDdeny,      OUTPUT);
  pinMode(LEDtakeOff,   OUTPUT);

  //Set up TRIGPINS for UltraSonic Device
  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  
  //Initialize Ethernet and account for 1s delay
  Ethernet.begin(mac, ip);
  delay(1000);

  Serial.println("connecting...");

  // Check if server connected or failed 
  // (Running express.js server at 5000 and react at 3000)
  if (client.connect(server, 5000)) {
    Serial.println("connected");
  } else {
    Serial.println("connection failed");
  }   

  STATE = 0;
  
}

void loop() {

    //State of rest. All variables cleared
    if(STATE == 0) {
      //Resets both RFID OFF and ON Outputs
      resetRFID();
      //Go to Zero State
      zeroState();
    }

    //Antenna lowers. Ultrasound turns on
    if(STATE == 1) {
      acceptedState();
    }

    //Denies and waits x seconds until reset to state 0
    if(STATE == 2) {
      deniedState();   
    }

    //Sends data to server about landing
    if(STATE == 3) {
      sendLandingDataState();  
    }

    //Landed (rest, wait until takeoff)
    if(STATE == 4) {
      landedState();  
    }

    //TakeOff Sequence (
    if(STATE == 5) {
      takeOffState();
    }
  
}



/**
 * When STATE == 0
 */
void zeroState() {
  
  //Make sure all other LEDS are off
  digitalWrite(LEDdeny, LOW);
  digitalWrite(LEDaccept, LOW);
  digitalWrite(LEDtakeOff, LOW);

  //Turn on zero-state LED
  digitalWrite(LEDzeroState, HIGH);


 
//  //Used to track state
//  buttonAcceptCounter  += digitalRead(buttonAccept);
//  buttonDenyCounter    += digitalRead(buttonDeny);
//
//  if(buttonAcceptCounter > 0) {
//    STATE = 1;
//  }
//
//  if(buttonDenyCounter > 0) {
//    STATE = 2;
//  }

  //ACCEPT (Drone 1)
  if(!digitalRead(bit1) && digitalRead(bit2)) {
      Serial.println(" 0 1 ");
       //Turn Off RFID
      turn_Off_RFID();
      STATE = 1;
  }

  //ACCEPT (Drone 2)
  if(digitalRead(bit1) && !digitalRead(bit2)) {
    Serial.println(" RFID OFF");
  }

  //Deny
  if(digitalRead(bit1) && digitalRead(bit2)) {
    Serial.println(" 1 1 ");
    //Turn Off RFID
    //turn_Off_RFID();
    STATE = 2;
  }

  if(!digitalRead(bit1) && !digitalRead(bit2)) {
    Serial.println(" 0 0 ");
  }







}


/**
 * When STATE == 1
 */
void acceptedState() {
  
  //Turn on the GREEN LED and Turn OFF WHITE LED
  digitalWrite(LEDzeroState, LOW);

  //Flash GREEN light
  digitalWrite(LEDaccept, HIGH);
  delay(50);
  digitalWrite(LEDaccept, LOW);
  delay(50);
  
  buttonAcceptCounter = 0;  //Reset button accepter counter

  //Authorize landing sequence
  landingSequence();

  if(landed == true) {
    Serial.print("Time it took to land: ");
    Serial.print(timeToLand);
    Serial.println(" seconds");
    STATE = 3;
  }

//  //Check for take off (stub with button)
//  buttonTakeOffCounter += digitalRead(takeOff);
//
//  if(buttonTakeOffCounter > 0) {
//    STATE = 3;
//  }
  
}

/**
 * State is when 
 */
void sendLandingDataState() {
  digitalWrite(LEDaccept, HIGH);

  //Send Landing Time to Server
  sendLandingTime("0", timeToLand);

  //When data is sent, go to state 4 (landed state)
  STATE = 4;
}

/**
 * When STATE == 4
 * Wait for takeoff
 */
void landedState() {

  //Start counter to see how long the drone stays for
  waitForTakeOff();

  if(landed == false) {
    Serial.print("Time it was parked: ");
    Serial.print(parkedTime);
    Serial.println(" seconds");
    
    //Wait for drone to leave, then initiate takeoff state after 5 seconds
    //Change the LEDs
    digitalWrite(LEDtakeOff, HIGH);
    digitalWrite(LEDaccept, LOW);
    delay(5000);
    turn_On_RFID();
    STATE = 5;
    
  }
}



/**
 * When STATE == 2
 */
void deniedState() {
  
  //Turn on the RED LED and Turn OFF WHITE LED
  digitalWrite(LEDdeny, HIGH);
  digitalWrite(LEDzeroState, LOW);
  buttonDenyCounter = 0;    //Reset button deny counter

  //Reset after two seconds (back to zero state)
  delay(2000);
  STATE = 0;
  
}


/**
 * When STATE == 5
 */
void takeOffState() {

  buttonTakeOffCounter = 0;   //Reset button takeoff counter
  sendParkedTime("0", parkedTime);
  
  //Simulate time it takes for Antenna to rise up
  delay(2000);
  STATE = 0;
  
}


/**
* Turn RFID Off
*/
void turn_Off_RFID() {
  Serial.println("Turning OFF RFID");
  digitalWrite(sendOff, HIGH);
  digitalWrite(sendOn, LOW);
}

/**
* Turn RFID On
*/
void turn_On_RFID() {
  Serial.println("Turning ON RFID");
  digitalWrite(sendOff, LOW);
  digitalWrite(sendOn, HIGH);
}

void resetRFID() {
  Serial.println("Resetting RFID");
  digitalWrite(sendOff, LOW);
  digitalWrite(sendOn, LOW);
}



/**
 * Constructs POST request
 * Sends TIMETOLAND to endpoint
 */
void sendLandingTime(String droneID, double timeToLand) {

    String data = "";
    data.concat("id=");
    data.concat(droneID); 
    data.concat("&"); 
    data.concat("timeToLand=");
    data.concat(timeToLand);

    
    Serial.println("Begin GET Request sendLandingTime");

    client.println("POST /routes/timeToLand HTTP/1.1");
    client.println("Host: localhost:5000"); 
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded"); 
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);   

    Serial.println("Sent Get Request sendLandingTime");
}


void sendParkedTime(String droneID, double parkedTime) {

  String data = "";
    data.concat("id=");
    data.concat(droneID); 
    data.concat("&"); 
    data.concat("parkedTime=");
    data.concat(parkedTime);

    
    Serial.println("Begin GET Request sendParkedTime");

    client.println("POST /routes/parkedTime HTTP/1.1");
    client.println("Host: localhost:5000"); 
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded"); 
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);   

    Serial.println("Sent Get Request sendParkedTime");
  
}




/**
 * Begins timer that ends when the drone has landed
 * Alters global variables 
 *    landed -> true
 *    timeToLand -> {time it took to land}
 */
void landingSequence() {

  //Write LOGIC for lowering antenna in different function

  //Initialize landing count (to be used to calculate time it takes to land)
  int landingCount;
  
  /**
   * TrigPin sends a signal and the ECHO pin picks it up
   * TrigPin goes LOW -> HIGH -> LOW
   */
  
  digitalWrite(TrigPin, LOW);       
  delayMicroseconds(2);
  digitalWrite(TrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);

  //The time it takes for it to return heps us calculate distance
  ECHOTIME = pulseIn(EchoPin, HIGH);
  cm = ECHOTIME / 58.0;               //The echo time is converted into cm
  cm = (int(cm * 100.0)) / 100.0;     //Keep two decimal places

  //Increment ultrasonic counter
  ultraSonicCount++;

  //Check if landed by seeing if the last 10 integers' average < 8cm
  if(ultraSonicCount > 10) {

    //Declare chunks (groups of 10 to check average)
    double ultraSonicChunk[10];
    double ultraSonicChunkTotal = 0;
    
    for(int i=0; i<10; i++) {
      ultraSonicChunk[i] = cm;
      ultraSonicChunkTotal += ultraSonicChunk[i];
    }

    //Check if landed by seeing if the last 10 integers' average < 8cm
    if((ultraSonicChunkTotal/10) < 8) {
      
      //Calculate time it takes to land (each delay is 250ms)
      landingCount = ultraSonicCount;
      double landingTime = landingCount * .250;

      //Re-initialize chunk, chunk size , and global variable ultrasonic count to 0
      memset(ultraSonicChunk, 0, sizeof(ultraSonicChunk));
      ultraSonicChunkTotal = 0;
      ultraSonicCount = 0;

      //Set GLOBAL variables
      landed = true;
      timeToLand = landingTime;
    }
  }

  //Run every quarter of a second
  delay(250);
}


/**
 * Begins timer that ends when the drone has taken off
 * Alters global variables 
 *    landed -> false
 *    parkedTime -> {time the drone took before it took off}
 */
void waitForTakeOff() {

  int parkedCount;
  
  /**
   * TrigPin sends a signal and the ECHO pin picks it up
   * TrigPin goes LOW -> HIGH -> LOW
   */
  
  digitalWrite(TrigPin, LOW);       
  delayMicroseconds(2);
  digitalWrite(TrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);

  //The time it takes for it to return heps us calculate distance
  ECHOTIME = pulseIn(EchoPin, HIGH);
  cm = ECHOTIME / 58.0;               //The echo time is converted into cm
  cm = (int(cm * 100.0)) / 100.0;     //Keep two decimal places

  //Increment ultrasonic counter
  ultraSonicCount++;

  //Check if landed by seeing if the last 10 integers' average < 8cm
  if(ultraSonicCount > 10) {
    
    //Declare chunks (groups of 10 to check average)
    double ultraSonicChunk[10];
    double ultraSonicChunkTotal = 0;
    
    for(int i=0; i<10; i++) {
      ultraSonicChunk[i] = cm;
      ultraSonicChunkTotal += ultraSonicChunk[i];
    }


    //Check if landed by seeing if the last 10 integers' average < 8cm
    if((ultraSonicChunkTotal/10) > 15) {
      
      //Calculate time it takes to land (each delay is 250ms)
      parkedCount = ultraSonicCount;
      double timeOfStay = parkedCount * .250;

      //Re-initialize chunk, chunk size , and global variable ultrasonic count to 0
      memset(ultraSonicChunk, 0, sizeof(ultraSonicChunk));
      ultraSonicChunkTotal = 0;
      ultraSonicCount = 0;

      //Set GLOBAL variables
      landed = false;
      parkedTime = timeOfStay;
    }

  }
    //Run every quarter of a second
    delay(250);
}
