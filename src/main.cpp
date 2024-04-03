/*yo need a 
led pin 
a stepper motor
esp32
ultrasonic sensor
buzzer
*/
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Stepper.h>
#include <FirebaseESP32.h>
#include <WiFi.h>
#include <string>



//Wifi configuration
const char* ssid = "wifi ssid";
const char* password = "passwod";

//Firbase configuration
#define API_KEY "api firebase"
#define DATABASE_URL "url"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;



const char* PARAM_INPUT_1 = "input1";



//declarations for stepper motor / ulltrasonic / buzzer /
#define ledPin 4
#define SOUND_SPEED 0.034
const int trigPin = 2;
const int echoPin = 15;
const int buzzer = 33;
long duration;
float distanceCm;
int  CarsNumber =0;
Stepper myStepper( 2048,25,27,26,14);

AsyncWebServer server(80);


//main page intrface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
<script>
    if (carExist) {
        document.write('<p>car exists</p>');
    }
</script>

  <title>Toggle LED</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    .button {
      display: inline-block;
      padding: 10px 20px;
      margin-right: 10px;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 5px;
      text-align: center;
      text-decoration: none;
      font-size: 16px;
    }
  </style>
</head>
<body>
  <p>
    <a href="/on"><button class="button">Add new Car</button></a>
    <a href="/out"><button class="button">car out</button></a>
    <a href="/off"><button class="button">Close</button></a>
  </p>
</body>
</html>)rawliteral";


//second page interface
const char indet_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
   <style>
    .button {
      display: inline-block;
      padding: 10px 20px;
      margin-right: 10px;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 5px;
      text-align: center;
      text-decoration: none;
      font-size: 16px;
    }
  </style>
  </head><body>
  <p>car entered successfully write her code</p>
  <form action="/get">
    input1: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
  <a href="/off"><button class="button">Close the barrier </button></a>
</body></html>)rawliteral";

bool carExist = false;

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  //wifia installation station mode
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
   pinMode(buzzer,OUTPUT);
   pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
   myStepper.setSpeed(5);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());


//main page opening
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
 
    request->send_P(200, "text/html", index_html);
  });


//the lgic when you clic in the buton add car
  server.on("/on", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if  (CarsNumber<4){
      CarsNumber++;
   myStepper.step(512);
    request->send_P(200, "text/html", indet_html);}
    else{
        request->send(200, "text/html", "<p>Sorry, parking is full of cars </p>" \
"<br><a href=\"/\">Return to Home Page</a>");

    }
  });

//when car get out it is decremented from cars number in the garage
    server.on("/out", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if  (CarsNumber>0){
      CarsNumber--;
   myStepper.step(512);
    request->send_P(200, "text/html", index_html);}
    
        request->send(200, "text/html", "<p>new car is out </p>" \
"<br><a href=\"/\">Return to Home Page</a>");

    
  });


//close the barrier
    server.on("/off", HTTP_GET, [] (AsyncWebServerRequest *request) {
    myStepper.step(-512);
     request->send_P(200, "text/html", index_html);
  });



//the logic to add the car  To firebase
 
   server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
  

       String s="/cars/"+inputMessage;
       Serial.println(s);
        if (Firebase.setString(fbdo,s,"entered")) {
      Serial.println("Int write PASSED");
    } else {
      Serial.println("Int write FAILED");
    
    }

    
      Serial.println("success");
    } else {
    

      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
+ inputParam + ") with value: " + inputMessage +
"<br><a href=\"/\">Return to Home Page</a>");
  });


  server.onNotFound(notFound);
  server.begin();

   /* Assign the RTDB URL (required) */
    config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

 
  

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
   
  }
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
     
}

void loop() {
    digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  

  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
if (distanceCm <5){
  digitalWrite(ledPin, HIGH);
  digitalWrite(buzzer,100);
  delay(1000);
}
else {
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzer,LOW);
    delay (1000);
}
  
  delay(5000);


}
