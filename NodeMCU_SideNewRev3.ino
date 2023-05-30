#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

// Credentials
//#define FIREBASE_HOST "digihydrodatabase-default-rtdb.firebaseio.com"
//#define FIREBASE_AUTH "2UQVdwfkemPRSRVCIctmcoFeQ1eDhRfTsUZfklGj"

#define FIREBASE_HOST "digihydro-c1d30-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "DVkgOFtOtcSMupwfOyygCInB2sj6e4mpx9hTVjvE"

#define try_WIFI_SSID "ZTE_2.4G_LW9scU"
#define try_WIFI_PASSWORD "X3N4cZW7"

#define WIFI_SSID "Mawian"
#define WIFI_PASSWORD "11282019"

SoftwareSerial NodeMCU_SoftSerial(D1, D2); 

String values;
String errorCode = "Critical Error: Sensors returning NAN values!";
String errorCode2 = "Critical Error: One sensor is not working";

int randomIDNumber;


//Time

#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

int timestamp;

void setup() {
  Serial.begin(115200); //Serial Communication | NodeMCU to PC
  
  NodeMCU_SoftSerial.begin(9600);  //Serial Communication | NodeMCU to Arduino
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting to SSID: Mawian");
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("try connecting to SSID: ZTE_2.4G_LW9scU");
    WiFi.begin(try_WIFI_SSID, try_WIFI_PASSWORD);
    delay(1000);
  }
  
  Serial.println();
  Serial.print(WiFi.SSID());
  Serial.print(" | connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // Init Time
  timeClient.begin();
}

void loop() {
  bool Sr = false;
 
  while(NodeMCU_SoftSerial.available()>0){
     values = NodeMCU_SoftSerial.readString();
     if(values.substring(0,8)  == "Critical"){
      
        Serial.println(errorCode);
        Firebase.setString("sensorStatus", errorCode);
        return;
        
     } else {
        Firebase.setString("sensorStatus", "Working Properly");
        Sr = true;
     }
     
  }

  delay(2000);
  
  if (Sr == true) {
    
   
    if(values.indexOf('E') != -1){
      Firebase.setString("unparsedData/dataValue", values.substring(0, values.indexOf('E')+5));
    } else {
      Firebase.setString("sensorStatus", errorCode2);
    }
    
    values = values.substring(0, values.indexOf('E')+5);
    
    int sliceAIndex = values.indexOf('A');
    int sliceBIndex = values.indexOf('B');
    int sliceCIndex = values.indexOf('C');
    int sliceDIndex = values.indexOf('D');
    int sliceEIndex = values.indexOf('E');
    
    String temperature = values.substring(sliceAIndex+1, sliceBIndex);
    String humidity = values.substring(sliceBIndex+1, sliceCIndex);
    String water_temperature = values.substring(sliceCIndex+1, sliceDIndex);
    String total_dissolved_solids = values.substring(sliceDIndex+1, sliceEIndex);
    String phValue = values.substring(sliceEIndex+1, sliceEIndex+5);
    
    if(total_dissolved_solids == "0.00E"){
      total_dissolved_solids = values.substring(sliceDIndex+1, sliceDIndex+5);
    } else {
      total_dissolved_solids = values.substring(sliceDIndex+1, sliceDIndex+6);
    }

    if (total_dissolved_solids.indexOf("E") != -1)
    {
        total_dissolved_solids.remove(total_dissolved_solids.indexOf("E"), 1);
    }
    
    delay(1000);
    
    Firebase.setString("Devices/0420/Temperature", temperature);

    Firebase.setString("Devices/0420/Humidity", humidity);

    Firebase.setString("Devices/0420/WaterTemperature", water_temperature);
    
    Firebase.setString("Devices/0420/TotalDissolvedSolids", total_dissolved_solids);

    Firebase.setString("Devices/0420/pH", phValue);
    
    delay(2000);
    
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);
    
    Firebase.setString("measurement_history/Devices/0420/"+ String(timestamp) +"/Temperature", temperature);
    Firebase.setString("measurement_history/Devices/0420/"+ String(timestamp) +"/Humidity", humidity);
    Firebase.setString("measurement_history/Devices/0420/"+ String(timestamp) +"/WaterTemperature", water_temperature);
    Firebase.setString("measurement_history/Devices/0420/"+ String(timestamp) +"/Tota", total_dissolved_solids);
    Firebase.setString("measurement_history/Devices/0420/"+ String(timestamp) +"/pH", phValue);
    Firebase.setString("measurement_history/Devices/0420/"+ String(timestamp) +"/timestamp", String(timestamp));

    delay(2000);
    
    if (Firebase.failed()) {
      Serial.flush();
      NodeMCU_SoftSerial.flush();
      return;
    }

    Serial.println("Data uploaded successfully");
    
  }

  
}

// Function that gets current epoch time
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}
