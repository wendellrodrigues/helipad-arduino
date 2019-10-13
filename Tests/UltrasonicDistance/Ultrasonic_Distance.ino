
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
double parkedTime = 0;

int STATE = 1;

void setup() {
  Serial.begin(9600);
  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);
}


void loop() {

  if(STATE == 1) {
    //Run landing sequence (will run in main program only on state change (RFID accepted))
    landingSequence();
  
    if(landed == true) {
      Serial.print("Time it took to land: ");
      Serial.print(timeToLand);
      Serial.println(" seconds");
      STATE = 2;
    }
 


  }

  if(STATE == 2) {

    waitForTakeOff();

    if(landed == false) {
      Serial.print("Time the vehicle was parked: ");
      Serial.print(parkedTime);
      Serial.println(" seconds");
      STATE = 0;
    }
    
  }
   
  
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
