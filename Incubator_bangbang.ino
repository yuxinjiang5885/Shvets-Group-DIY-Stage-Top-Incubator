// Include Libraries
#include "math.h"
#include "SoftwareSerial.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>

// Define Constants
// #define         MG_PIN                       (A0)    //define which analog input channel for CO2 sensor
#define         HEATER_PIN                   (2)     //define pin to control heater
// #define         DC_GAIN                      (8.5)   //define the DC gain of amplifier
// #define         SOLENOID_PIN                 (3)     //define pin to control CO2 input
// #define         DHTPIN                       (4)     //define pin for temperature and humditiy sensor
// #define         DHTTYPE                      (DHT22) //indicate we're using a DHT22/AM2302 temp/humidity sensor
#define         ATOMIZER_PIN                 (7)     //pin to drive atomizer

// #define         READ_SAMPLE_INTERVAL         (50)    //define the time interval(in milisecond) between each samples in normal operation
// #define         READ_SAMPLE_TIMES            (20)     //define how many samples you are going to take in normal operation
// #define         ZERO_POINT_VOLTAGE           (0.220) //define the output of the sensor in volts when the concentration of CO2 is 400PPM
// #define         REACTION_VOLTAGE             (0.030) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2
// #define         VOLTAGE_THRESHOLD            (0.9)

// Creat new sensor object
Adafruit_AM2320 am2320 = Adafruit_AM2320();


// //Sets up a virtual serial port using pin 12 for Rx and pin 13 for Tx
// SoftwareSerial K_30_Serial(12,13);


// // Calibration curve for CO2 sensor
// float           CO2Curve[3]  =  {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE/(2.602-3))};
// float           CO2_CURVE[2] = {173.04, -4.652};

// Variables for communicating with lab computer
String data;
char d1;        // Input from lab PC


// Environmental Set Values
const float SET_TEMP = 30; // Original 37
const float SET_HUMIDITY = 40; // Original 90
// const float SET_PERCENTAGE = 5.25;
// const float SET_VOLTAGE = 0.7;
const float TEMP_OFFSET = 0.22;
const float HUMID_OFFSET = 1.57;


//initializing variables
float checkTemp = 0.0;
float checkHumid = 0.0;
float temp = 0.0;
float humidity = 0.0;
// float percentage = 0.0;
// float CO2percentage = 0.0;
// float CO2sum = 0.0;
// float CO2average = 0.0;
// int CO2counts = 0;
// float volts = 0;

// Variables for handling heater bursts
unsigned long heaterOnTime;
unsigned long heaterOffTime;
unsigned long heaterInterval = 60000;
unsigned long heaterBurst = 300000;
bool heater = false;                          // Records heater state
bool heaterStandby = true;                   // Records whether the user want to control temp


// // Variables for handling solenoid bursts
// unsigned long solenoidOnTime;
// unsigned long solenoidOffTime;
// unsigned long solenoidInterval = 30000;
// unsigned long solenoidBurst = 1000;
// bool solenoid = false;                        // Records solenoid state
// bool solenoidStandby = true;                 // Records whether user wants to control CO2

// Variables for handling atomizer
bool atomizer = false;                        // Records atomizer state
bool atomizerStandby = true;                 // Records whether user wants to control humidity

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // K_30_Serial.begin(9600); 
  pinMode(HEATER_PIN, OUTPUT);
  // pinMode(SOLENOID_PIN, OUTPUT);
  pinMode(ATOMIZER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  // digitalWrite(SOLENOID_PIN, LOW);
  // digitalWrite(ATOMIZER_PIN, LOW); // Default is low
  digitalWrite(ATOMIZER_PIN, HIGH); // Default is high

  // // Initialize Temp and humidity sensor
  // dht.begin();
  // temp = dht.readTemperature();
  // humidity = dht.readHumidity();
  Wire.begin();               
  am2320.begin(); 
}


void loop() {
 
  // Communication with the lab computer
  if(Serial.available()){
    data = Serial.readString();
    d1 = data.charAt(0);
    if(d1=='A'){
      heaterStandby = false;
    }
    else if(d1=='a'){
      heaterStandby = true;
    }
    // else if(d1=='B'){
    //   solenoidStandby = false;
    // }
    // else if(d1=='b'){
    //   solenoidStandby = true;
    // }
    else if(d1=='C'){
      atomizerStandby = false;
    }
    else if(d1=='c'){
      atomizerStandby = true;
    }
  }


  // // Check CO2
  // volts = MGRead(MG_PIN);
  // percentage = MGGetPercentage(volts, CO2Curve);
  // CO2percentage = CO2_CURVE[0]*exp(CO2_CURVE[1]*volts);
  // CO2sum = CO2sum + CO2percentage;
  // CO2counts = CO2counts + 1;
  // CO2average = CO2sum/CO2counts;

  // // If the solenoid is open, check how long it has been open
  // if(solenoidStandby == false){
  //   if(solenoid){
  //     // If it has been open for longer than solenoidBurst, close it
  //     if((millis() - solenoidOnTime) >= solenoidBurst){
  //       digitalWrite(SOLENOID_PIN, LOW);
  //       solenoid = false;
  //       solenoidOffTime = millis();
  //     }
  //   }
  //   // If CO2 is too low and the solenoid has been closed longer than the interval (and is active), open solenoid and record time
  //   else if((CO2average < SET_PERCENTAGE) && ((millis() - solenoidOffTime) >= solenoidInterval)){
  //     digitalWrite(SOLENOID_PIN,HIGH);
  //     solenoidOnTime = millis();
  //     solenoid = true;
  //     CO2sum = 0.0;
  //     CO2counts = 0;
  //   }
  // }
  // else if(solenoid){
  //   digitalWrite(SOLENOID_PIN, LOW);
  //   solenoid = false;
  // }


  // Check temperature
  // checkTemp = dht.readTemperature();
  checkTemp = am2320.readTemperature();
  // Serial.print("checkTemp: ");Serial.println(checkTemp); // Debug
  if(!isnan(checkTemp)){
    temp = checkTemp;
    // Serial.print("Debug temp: ");Serial.println(temp); // Debug
      // Check whether or not heater control is on 
    if(heaterStandby == false){
        if(heater){
          if(temp > (SET_TEMP-TEMP_OFFSET)){
            digitalWrite(HEATER_PIN, LOW);
            heater = false;
          }
        }
        else if(temp <= (SET_TEMP-TEMP_OFFSET)){
          Serial.println("Heater on");
          digitalWrite(HEATER_PIN, HIGH);
          heater = true;
        }
    }
    else if(heater){
      digitalWrite(HEATER_PIN, LOW);
      heater = false;
    }
  }


  // // Check humidity
  // // checkHumid = dht.readHumidity();
  // checkHumid = am2320.readHumidity();     // %RH
  // if(!isnan(checkHumid)){
  //   humidity = checkHumid;
  //   // If humidity is too low, turn on the atomizer
  //   if(atomizerStandby == false){
  //     if(atomizer){
  //       if(humidity >= SET_HUMIDITY){
  //         digitalWrite(ATOMIZER_PIN, LOW);
  //         atomizer = false;
  //       }
  //     }
  //     else if(humidity < (SET_HUMIDITY-HUMID_OFFSET)){
  //       Serial.println("Humidifier on");
  //       digitalWrite(ATOMIZER_PIN, HIGH);
  //       atomizer = true;
  //     }
  //     else{
  //       digitalWrite(ATOMIZER_PIN, LOW);
  //       atomizer = false;
  //     }
  //   }
  //   else if(atomizer){
  //     digitalWrite(ATOMIZER_PIN, LOW);
  //     atomizer = false;
  //   }

  // }

  // Backup: Check humidity
  // checkHumid = dht.readHumidity();
  checkHumid = am2320.readHumidity();     // %RH
  if(!isnan(checkHumid)){
    humidity = checkHumid;
    // digitalWrite(ATOMIZER_PIN, HIGH);
    // If humidity is too low, turn on the atomizer
    if(atomizerStandby == false){ // If the atomizer is not standby, controlled by "C"
      if(atomizer){// Humidifier is on
        if(humidity >= SET_HUMIDITY){ // Humidifier is on & humidity exceeds threshold, turn humidifier off
          // digitalWrite(ATOMIZER_PIN, HIGH); // Default is low
          // delay(1000);
          // digitalWrite(ATOMIZER_PIN, LOW); 
          digitalWrite(ATOMIZER_PIN, LOW); // Default is high
          delay(100);
          digitalWrite(ATOMIZER_PIN, HIGH); 
          atomizer = false;
        }
      }
      // else if(humidity < (SET_HUMIDITY-HUMID_OFFSET)){ // Humidifier is on & humidifier not yet at threshold, keep it on
      //   // Serial.println("Humidifier on");
      //   // digitalWrite(ATOMIZER_PIN, LOW);
      //   atomizer = true;
      // }
      else{ // Humidifier is off
        if(humidity < (SET_HUMIDITY)){ // Humidifier is on & humidity is not yet at threshold, turn humidifier on
          // digitalWrite(ATOMIZER_PIN, HIGH); // Default is low
          // delay(1000);
          // digitalWrite(ATOMIZER_PIN, LOW); 
          digitalWrite(ATOMIZER_PIN, LOW); // Default is high
          delay(100);
          digitalWrite(ATOMIZER_PIN, HIGH); 
          atomizer = true;
        }

      }
    }
    else{ // If the atomizer is standby, controlled by "c"
      if(atomizer){
        // digitalWrite(ATOMIZER_PIN, HIGH); // Default is low
        // delay(1000);
        // digitalWrite(ATOMIZER_PIN, LOW); 
          digitalWrite(ATOMIZER_PIN, LOW); // Default is high
          delay(100);
          digitalWrite(ATOMIZER_PIN, HIGH); 
      }
      else{
        // digitalWrite(ATOMIZER_PIN, LOW); // Default is low
        digitalWrite(ATOMIZER_PIN, HIGH); // Default is high
      }
      atomizer = false;
    }
  }


// Output to exe
  // Serial.print("p");
  // Serial.println(CO2percentage);
  Serial.print("Temp: ");
  Serial.println(temp);
  Serial.print("Hum: ");
  Serial.println(humidity);
  
  Serial.print("Humidifier state: "); Serial.println(atomizer);
  Serial.print("Heater state: "); Serial.println(heater);


/*
// Output to IDE
  // Output sensor readings
  Serial.print(F(" Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  "));
  Serial.print(atomizer);
  Serial.print(F("  Temperature: "));
  Serial.print(temp);
  Serial.print(F("°C  "));
  Serial.print(heater);
  Serial.print(F("  CO2: "));
  Serial.print(CO2percentage);
  Serial.print(F("% "));
  Serial.print(F("  CO2avg: "));
  Serial.print(CO2average);
  Serial.print(F("% "));
  Serial.println(solenoid);
*/

  // Wait between loops
  delay(1000);
}


// // Method Definitions
// float MGRead(int mg_pin)
// {
//     int i;
//     float v=0;

//     for (i=0;i<READ_SAMPLE_TIMES;i++) {
//         v += analogRead(mg_pin);
//         delay(READ_SAMPLE_INTERVAL);
//     }
//     v = (v/READ_SAMPLE_TIMES) *5/1024 ;
//     return v;
// }

// int  MGGetPercentage(float volts, float *pcurve)
// {
//    if ((volts/DC_GAIN) >= ZERO_POINT_VOLTAGE) {
//       return -1;
//    } else {
//       return pow(10, ((volts/DC_GAIN)-pcurve[1])/pcurve[2]+pcurve[0]);
//    }
// }
