#include <ArduinoJson.h>
#include <Ethernet.h>
#include <SPI.h>

//WORKING MODEL OF SENDING DATA TO SERVER

byte mac[]      = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   //Self-Assigned MAC address
byte ip[]       = { 10, 0, 0, 177 };                        //Self-Assigned IP for arduino
byte server[]   = { 10, 0, 0, 248 };                        //IP of laptop (where server is locally hosted)

bool stopLoop = false;

EthernetClient client;
                        
void setup()
{
  //Initialize Ethernet and account for 1s delay
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
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

  if(stopLoop == false) {

    String data = "";
    data.concat("name=");
    data.concat("Bill"); 
    data.concat("&"); 
    data.concat("timeToLand=");
    data.concat(20);
    data.concat("&");
    data.concat("parkedTime=");
    data.concat(50);
    
    Serial.println("Begin GET Request");

    client.println("POST /routes/test HTTP/1.1");
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

  stopLoop = true;


}
