
// ADDING THE LIBRARIES
#include <Arduino.h>  // the platformio and boards communication 
#include <TinyGPSPlus.h> // for the gps sensor
#include <Firebase.h>  // the firebase libraries for the IoT communication
#include <Firebase_ESP_Client.h>
#include <WiFi.h>

// Creating the credentials of the system....

//======THE-ESP32-IOT-CONFIGURATION=======
// 1. The WiFi Credentials....
#define WIFI_SSID "matrix technology"
#define WIFI_PASSWORD "frumark=115"

// 2. The Firebase credentials.....
FirebaseData swimsDB;  // the database object
FirebaseAuth swimsAuth; // the user_sign in object
FirebaseConfig swimsConfig; // the configuration object

unsigned long sendDataTimer = 0;  // the send data timer...

// the firebase project api key, this needs to be changed.....
#define API_KEY "AIzaSyC4_I5CFFz3cAa2uB2Jo1lQs8m8m1oT0IE"
// the database url ..... needs to be changed
#define DB_URL "https://swims-web-app-c6097-default-rtdb.firebaseio.com" 


// creating the sign in credentials...
// the credentials do not need to be hard coded into the code but needs to be the user input
#define USER_EMAIL "frucarrington@gmail.com"
#define USER_PASSWORD "markchei"

// 3. Defining the Serial communication baudRates
#define gpsSerialBaudRate 9600 // for the gps sensor
#define normalBaudRate 115200 // for the serial monitor

// 4. Defining the pins for the Serial port and the waste level sensor...
int trig = 26, echo = 36; // the ultrasonic sensor pins

// the ultrasonic sensor variables
float distance, duration; // the durationa and distance variables
float conversion = 29/2; // this will display distance in cm

// Defining the TinyGPSPlus and SoftwareSerial instances....
TinyGPSPlus gps;

// initialize the WiFi communication....
void wifi_init(){
  // beginning the WiFi communication.....
  // to be replaced by the WiFiManager library of the ESP32......
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  // testing the communication connection

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("the network has not connected");

  }
  Serial.println("The connection has been established");
}


// Everything about the firebase updates on this code will be redone and retyped in another code.


void Esp32_setup(){
  // setting the signing in credentials....
  swimsAuth.user.email = USER_EMAIL;
  swimsAuth.user.password = USER_PASSWORD; 

  // setting up the api key and the db
  swimsConfig.api_key = API_KEY;
  swimsConfig.database_url = DB_URL;

  // beginning the firebase communication....
  Firebase.begin(&swimsConfig, &swimsAuth);
  Firebase.reconnectWiFi(true);
}

void setup() {
  Serial.begin(normalBaudRate);
  Serial2.begin(gpsSerialBaudRate, SERIAL_8N1, 16, 17);

  // initializing the WiFi communication
  wifi_init();
  Esp32_setup();

  // setting up ultrasonic sensor pins
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
}

// receiving the location data....

// 1. the gps latitude...
float gpsLatitude(){
  if (gps.location.isUpdated() && gps.location.isValid()){
    float latitude = (gps.location.lat(), 6);
    Serial.print("latitude: ");
    Serial.println(latitude);
    return latitude;
  }
}

// 2. the gps longtitude.....
float gpsLongtitude(){
  if (gps.location.isUpdated() && gps.location.isValid()){
    float longtitude = (gps.location.lng(), 6);
    Serial.print("longtitude: ");
    Serial.println(longtitude);
    return longtitude;
  }
}

// the ultrasonic sensor data
float readingDistance(int trigPin, int echoPin){

  // SENDING OUT THE WAVE PULSE
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin, LOW);

  // getting the bounced wave
  duration = pulseIn(echoPin, HIGH);
  distance = duration/conversion;
  return distance;  

}



void firebaseSendData(){ // this part sends the data to the real time db of firebase
  
  if (Firebase.ready() && millis() - sendDataTimer > 15000 || sendDataTimer == 0){
    sendDataTimer = millis();
    // sending the data....
    //1. The location data.
    while (Serial2.available()){
      if(gps.encode(Serial2.read())){
        // we can read the gps signal here..
        if(gps.location.isValid()){
          float received_lat = gpsLatitude();
          float received_long = gpsLongtitude();
          Firebase.RTDB.setFloat(&swimsDB, 'swims/bindata/latitude',  received_lat);
          Firebase.RTDB.setFloat(&swimsDB, 'swims/bindata/longtitude',  received_long);
        }

    }
  }
  // 2. The waste level data
  float distance = readingDistance(trig, echo);
  Firebase.RTDB.setFloat(&swimsDB, 'swims/bindata/wastelevel',  distance);

  }
}

void loop() {
  firebaseSendData();
  delay(1000);
  
}