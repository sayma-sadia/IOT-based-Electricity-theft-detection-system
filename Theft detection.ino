#include <SoftwareSerial.h>

SoftwareSerial sim808(7,8);

char phone_no[] = "01868391381";

String data[5];

#define DEBUG true

String state,timegps,latitude,longitude;

#include "ACS712.h"

#include <Wire.h>

#include <LiquidCrystal_I2C.h>

ACS712 sensor1(ACS712_20A, A0);

ACS712 sensor2(ACS712_20A, A1);

LiquidCrystal_I2C lcd(0x3F, 16, 2);

void setup() {
  Serial.begin(9600);
  sim808.begin(9600);
  delay(50);
  sim808.print("AT+CSMP=17,167,0,0"); // set this parameter if empty SMS received
  delay(100);
  sim808.print("AT+CMGF=1\r");
  delay(400);
  sendData("AT+CGNSPWR=1",1000,DEBUG);
  delay(50);
  sendData("AT+CGNSSEQ=RMC",1000,DEBUG);
  delay(150);
  sensor1.calibrate();
  sensor2.calibrate();
  lcd.begin();
}

void loop() {
  float current1 = sensor1.getCurrentAC();
  float current2 = sensor2.getCurrentAC();

  // Threshold checks
  if (current1 < 0.15) {
    current1 = 0;
  }
  if (current2 < 0.08) {
    current2 = 0;
  }
  
  // Current value adjustments
  if (current1 > 0.20) {
    if(current1 > 0.4) {
      if(current1>0.5) {
        current1=0.50;
      } else {
        current1=0.40;
      }
    } else {
      current1 = 0.24;
    }
  }
  if (current2 > 0.20) {
    if(current2 > 0.4) {
      if(current2>0.5) {
        current2=0.50;
      } else {
        current2=0.40;
      }
    } else {
      current2 = 0.24;
    }
  }

  // Theft detection condition
  if(current1>current2) {
    Serial.println(" Theft Detected");
    lcd.clear();
    lcd.print("Theft Detected");
    delay(1000);
    sendTabData("AT+CGNSINF",1000,DEBUG);
    Serial.println(" Time:"+timegps);
    delay(3000);
    Serial.println(" Latitude:"+latitude);
    delay(5000);
    Serial.println(" Longitude:"+longitude);

    // Send SMS with location
    sim808.print("AT+CMGS=\"");
    sim808.print(phone_no);
    sim808.println("\"");
    delay(300);
    sim808.print("http://maps.google.com/maps?q=loc:");
    sim808.print(latitude);
    sim808.print(",");
    sim808.print(longitude);
    delay(200);
    sim808.println((char)26);  // End of message character
    delay(200);
    sim808.println();
    delay(20000);
    sim808.flush();
    delay(1000);
  }
  else {
    Serial.println(" No Theft");
    lcd.clear();
    lcd.print(" No Theft");
    lcd.setCursor(3,1);
    lcd.print("Detected");
    delay(2000);
  }
  delay(5000);
}

void sendTabData(String command , const int timeout , boolean debug){
  sim808.println(command);
  long int time = millis();
  int i = 0;
  while((time+timeout) > millis()){
    while(sim808.available()){
      char c = sim808.read();
      if (c != ',') {
        data[i] +=c;
        delay(100);
      }
      else {
        i++;
      }
      if (i == 5) {
        delay(100);
        goto exitL;
      }
    }
  }
  exitL:
  if (debug) {
    state = data[1];
    timegps = data[2];
    latitude = data[3];
    longitude =data[4];
  }
}

String sendData (String command , const int timeout ,boolean debug){
  String response = "";
  sim808.println(command);
  long int time = millis();
  while ( (time+timeout ) > millis()){
    while (sim808.available()){
      char c = sim808.read();
      response +=c;
    }
  }
  if (debug) {
    Serial.print(response);
  }
  return response;
}
