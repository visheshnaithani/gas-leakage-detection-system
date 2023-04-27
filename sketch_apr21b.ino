#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

/* Callback function to get the Email sending status */
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
const char* ssid = "vish";
const char* password = "8447269029";
String phoneNumber = "918447269029";
String apiKey = "6807677";
// String readString;
#define API_KEY "AIzaSyAK2GXjtQ09E0V-_XyPevrWYC7Mm94YI0o"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "visheshn2002@gmail.com"
#define USER_PASSWORD "gulabjamun"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://leakage-detection-2131c-default-rtdb.firebaseio.com/"
#define RXp2 16
#define TXp2 17
// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
String lpgPath = "/lpg";
String coPath = "/co";
String smokePath = "/smoke";
String statusPath = "/status";
String timePath = "/timestamp";


// Parent Node (to be updated in every loop)
String parentPath;

FirebaseJson json;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
// Variable to save current epoch time
int timestamp;
// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 100-0;
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}
void sendMessage(String message){

  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    
  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
    Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}
void loop() {
     if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
     }
    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);
    String lpg, co, smoke, status;
    int i1, i2, i3;
    int j = 0;
    String readString = Serial2.readStringUntil('/n');
    for(int i = 0; i < readString.length(); i++){
      if(readString[i] == ',' && j == 0){
        i1 = i;
        j++;
      }
      else if(readString[i] == ',' && j == 1){
        i2 = i;
        j++;
      }
      else if(readString[i] == ',' && j == 2){
        i3 = i;
        j++;
      }
    }
    lpg = readString.substring(0, i1);
    co = readString.substring(i1 + 1, i2);
    smoke = readString.substring(i2 + 1, i3);
    status = readString.substring(i3 + 1, readString.length()-1);
    Serial.println(status);
    int x = status.toInt();
    if(x == 1){
      sendMessage("Leakage Detected!!!");
      Serial.println("Leakage");
    }
    json.set(lpgPath.c_str(), lpg);
    json.set(coPath.c_str(), co);
    json.set(smokePath.c_str(), smoke);
    json.set(statusPath.c_str(), String(x));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    readString = "";
    delay(5000);

}