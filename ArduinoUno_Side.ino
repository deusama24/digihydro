#include <SoftwareSerial.h>
SoftwareSerial ArduinoUno_SoftSerial(10, 11);

//DHT22 SENSOR
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// DS18B20 - WATER TEMPERATURE SENSOR
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature DSsensors(&oneWire);

//TOTAL DISSOLVED SOLID SENSOR
#define TdsSensorPin A1 
#define VREF 5.0
              
#define SCOUNT  30 // Sum Sample Point        

//TOTAL DISSOLVED SOLID SENSOR Variables 
int analogBuffer[SCOUNT];   
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float basetemperature = 25; // Temperature Baseline

//pH SENSOR
#include <Wire.h>
float calibration_value = 19.34;
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;

float ph_act;
String values;



void setup() {
  Serial.begin(115200); //Serial Communication | ArduinoUno to PC

  ArduinoUno_SoftSerial.begin(9600); //Serial Communication | Arduino to NodeMCU
  // Initialize DHT22 
  dht.begin();

  // Initialize DS18B20 
  DSsensors.begin();

}

void loop() {
  totaldissolvesolids_sensor(); 
  float humidity  = dht.readHumidity();
  float temperature = dht.readTemperature();
  DSsensors.requestTemperatures(); 
  float waterTemp = DSsensors.getTempCByIndex(0);
  
  if(isnan(humidity) || isnan(temperature) || isnan(waterTemp) || waterTemp == -127.00 ) {
    Serial.println("Critical Error: Sensors returning NAN values!");
    ArduinoUno_SoftSerial.print("Critical Error: Sensors returning NAN values!");
    
    delay(1000);
    return;
  }
  
  
  if (waterTemp == -127.00) {
    waterTemp = 0;
    basetemperature = 25;
  } else {
    basetemperature = waterTemp;
  }

  Serial.print("Water Temperature: ");
  Serial.println(waterTemp);
  
  Serial.print("TDS Value:");
  Serial.print(tdsValue);
  Serial.println("ppm");
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("  |  "); 
  Serial.print("Temperature: ");
  Serial.println(temperature);

  Serial.print("pH Sensor: ");
  Serial.println(phSensor());

  Serial.flush();
  delay(1000);
  
  values = ("A" + String(temperature) + "B" + String(humidity) + "C" + String(waterTemp) + "D" + String(tdsValue) + "E" + String(ph_act));

  Serial.println(values);
  ArduinoUno_SoftSerial.print(values);
  delay(1000);
}
  


float phSensor(){
   for(int i=0;i<10;i++) 
   { 
     buffer_arr[i]=analogRead(A3);
     delay(30);
   }
   for(int i=0;i<9;i++)
   {
     for(int j=i+1;j<10;j++)
     {
       if(buffer_arr[i]>buffer_arr[j])
       {
         temp=buffer_arr[i];
         buffer_arr[i]=buffer_arr[j];
         buffer_arr[j]=temp;
       }
     }
   }
   avgval=0;
   for(int i=2;i<8;i++)
     avgval+=buffer_arr[i];
     float volt=(float)avgval*5.0/1024/6;
     ph_act = -5.20 * volt + calibration_value;
     return ph_act;

}

void totaldissolvesolids_sensor(){
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
        averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
        float compensationCoefficient=1.0+0.02*(basetemperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
        float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
        tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
   }
}

// MEDIAN FILTERING ALGORITHM
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}
