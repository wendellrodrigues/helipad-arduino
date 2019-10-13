



//Output to turn RFID Off
//int sendRFID_OFFsignal = 53;  //RFID 7 -> MAIN 53

//Inputs
int bit1      = 30;   //RFID 4  -> MAIN 30
int bit2      = 31;   //RFID 5  -> MAIN 31
int sendOff   = 32;   //MAIN 32 -> RFID 6
int sendOn    = 33;   //MAIN 33 -> RFID 7

int state = 0;

void setup() {
  Serial.begin(9600);
  pinMode(bit1,                         INPUT);
  pinMode(bit2,                         INPUT);
  pinMode(sendOff,                      OUTPUT);
  pinMode(sendOn,                       OUTPUT);

}

void loop() {

  if(state == 0) {
    
    if(!digitalRead(bit1) && digitalRead(bit2)) {
      Serial.println(" 0 1 ");
      state = 1;
    }

    //RFID Off
    if(digitalRead(bit1) && !digitalRead(bit2)) {
      Serial.println(" RFID OFF");
    }

    if(digitalRead(bit1) && digitalRead(bit2)) {
      Serial.println(" 1 1 ");
    }

    if(!digitalRead(bit1) && !digitalRead(bit2)) {
      Serial.println(" 0 0 ");
    }
    
  }

  if(state == 1) {
    Serial.println("TURNING OFF RFID");
    turn_Off_RFID();
    state = 2;
  }


  if(state == 2) {
    Serial.println("TURNING ON RFID");
    delay(2000);
    turn_On_RFID();
    state = 3;   
  }

  if(state == 3) {
    delay(2000);
    state = 0;
  }

  

}

/**
 * Turn RFID Off
 */
 void turn_Off_RFID() {
    digitalWrite(sendOff, HIGH);
    digitalWrite(sendOn, LOW);
 }

 /**
 * Turn RFID On
 */
 void turn_On_RFID() {
    digitalWrite(sendOff, LOW);
    digitalWrite(sendOn, HIGH);
 }





























/**
 * 
    Serial.begin(9600);

    //Check for accept1, accept2, deny
    int accept1 = digitalRead(accept_user1);
    int accept2 = digitalRead(accept_user2);
    int deny    = digitalRead(deny);

    if(accept1 == 1) {
      state = 1;
    }

    if(accept2 == 1) {
      state = 2;
    }

//    if(deny == 1) {
//      state = 3;
//    }  
    
  }

  if(state == 1) {
    delay(2000);
    state = 4;
  }




  if(state == 4) {
    digitalWrite(sendRFID_OFFsignal, HIGH);
  }

  Serial.println(state);
 */
