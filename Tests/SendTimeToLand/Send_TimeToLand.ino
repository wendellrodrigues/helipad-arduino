#include <Ethernet.h>
#include <SPI.h>

//Variables for server connection
byte mac[]      = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   //Self-Assigned MAC address
byte ip[]       = { 10, 0, 0, 177 };                        //Self-Assigned IP for arduino
byte server[]   = { 10, 0, 0, 248 };                        //IP of laptop (where server is locally hosted)

//Initialize Trigger and Echo Pins
//Other 2 pins (VCC and GND) go to +5V and GND, respectively
const int TrigPin = 2;
const int EchoPin = 3;

//Declare two variables
float cm;
float ECHOTIME;

//Declare a count
int ultraSonicCount = 0;

//Landed variables
bool landed = false;
double timeToLand = 0; //Time it takes to land

int STATE = 1;

EthernetClient client;
                        
void setup()
{

  Serial.begin(9600);

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
}



void loop() {

  if(STATE == 1) {
    landingSequence();
  }
  //Run landing sequence (will run in main program only on state change (RFID accepted))
  

  if(landed == true) {
    Serial.print("Time it took to land: ");
    Serial.print(timeToLand);
    Serial.println(" seconds");
    STATE = 2;
  }

  //Reset to false;
  landed = false;

  if(STATE == 2) {
    sendLandingTime("123", timeToLand);
    STATE = 3;
  }

  
   
  

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

    
    Serial.println("Begin GET Request");

    client.println("POST /routes/timeToLand HTTP/1.1");
    client.println("Host: localhost:5000"); 
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded"); 
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);   

    Serial.println("Sent Get Request");
}


/**
 * Begins timer that ends when the drone has landed
 * Alters global variables 
 *    landed -> true
 *    timeToLand -> {time it took to land}
 */
void landingSequence() {

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
