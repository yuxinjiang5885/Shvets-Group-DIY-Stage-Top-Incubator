// Include Libraries
#include "math.h"
#include "SoftwareSerial.h"
#include "DHT.h"

// Define Constants
#define         MG_PIN                       (A0)    //define which analog input channel for CO2 sensor
#define         HEATER_PIN                   (2)     //define pin to control heater
#define         DC_GAIN                      (8.5)   //define the DC gain of amplifier
#define         SOLENOID_PIN                 (3)     //define pin to control CO2 input
#define         DHTPIN                       (4)     //define pin for temperature and humditiy sensor
#define         DHTTYPE                      (DHT22) //indicate we're using a DHT22/AM2302 temp/humidity sensor
#define         ATOMIZER_PIN                 (5)     //pin to drive atomizer

#define         READ_SAMPLE_INTERVAL         (50)    //define the time interval(in milisecond) between each samples in normal operation
#define         READ_SAMPLE_TIMES            (20)     //define how many samples you are going to take in normal operation
#define         ZERO_POINT_VOLTAGE           (0.220) //define the output of the sensor in volts when the concentration of CO2 is 400PPM
#define         REACTION_VOLTAGE             (0.030) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2
#define         VOLTAGE_THRESHOLD            (0.9)

// Define DHT object to read temp and humidity
DHT dht(DHTPIN, DHTTYPE);


//Sets up a virtual serial port using pin 12 for Rx and pin 13 for Tx
SoftwareSerial K_30_Serial(12,13);


// Calibration curve for CO2 sensor
float           CO2Curve[3]  =  {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE/(2.602-3))};
float           CO2_CURVE[2] = {173.04, -4.652};

// Variables for communicating with lab computer
String data;
char d1;        // Input from lab PC


// Environmental Set Values
const float SET_TEMP = 37;
const float SET_HUMIDITY = 90;
const float SET_PERCENTAGE = 5.25;
const float SET_VOLTAGE = 0.7;
const float TEMP_OFFSET = 0.22;
const float HUMID_OFFSET = 1.57;


//initializing variables
float checkTemp = 0.0;
float checkHumid = 0.0;
float temp = 0.0;
float humidity = 0.0;
float percentage = 0.0;
float CO2percentage = 0.0;
float CO2sum = 0.0;
float CO2average = 0.0;
int CO2counts = 0;
float volts = 0;

// Variables for handling heater bursts
unsigned long heaterOnTime;
unsigned long heaterOffTime;
unsigned long heaterInterval = 60000;
unsigned long heaterBurst = 300000;
bool heater = false;                          // Records heater state
bool heaterStandby = true;                   // Records whether the user want to control temp


// Variables for handling solenoid bursts
unsigned long solenoidOnTime;
unsigned long solenoidOffTime;
unsigned long solenoidInterval = 30000;
unsigned long solenoidBurst = 1000;
bool solenoid = false;                        // Records solenoid state
bool solenoidStandby = true;                 // Records whether user wants to control CO2

// Variables for handling atomizer
bool atomizer = false;                        // Records atomizer state
bool atomizerStandby = true;                 // Records whether user wants to control humidity



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  K_30_Serial.begin(9600); 
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);
  pinMode(ATOMIZER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(SOLENOID_PIN, LOW);
  digitalWrite(ATOMIZER_PIN, LOW);

  // Initialize Temp and humidity sensor
  dht.begin();
  temp = dht.readTemperature();
  humidity = dht.readHumidity();
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
    else if(d1=='B'){
      solenoidStandby = false;
    }
    else if(d1=='b'){
      solenoidStandby = true;
    }
    else if(d1=='C'){
      atomizerStandby = false;
    }
    else if(d1=='c'){
      atomizerStandby = true;
    }
  }


  // Check CO2
  volts = MGRead(MG_PIN);
  percentage = MGGetPercentage(volts, CO2Curve);
  CO2percentage = CO2_CURVE[0]*exp(CO2_CURVE[1]*volts);
  CO2sum = CO2sum + CO2percentage;
  CO2counts = CO2counts + 1;
  CO2average = CO2sum/CO2counts;

  // If the solenoid is open, check how long it has been open
  if(solenoidStandby == false){
    if(solenoid){
      // If it has been open for longer than solenoidBurst, close it
      if((millis() - solenoidOnTime) >= solenoidBurst){
        digitalWrite(SOLENOID_PIN, LOW);
        solenoid = false;
        solenoidOffTime = millis();
      }
    }
    // If CO2 is too low and the solenoid has been closed longer than the interval (and is active), open solenoid and record time
    else if((CO2average < SET_PERCENTAGE) && ((millis() - solenoidOffTime) >= solenoidInterval)){
      digitalWrite(SOLENOID_PIN,HIGH);
      solenoidOnTime = millis();
      solenoid = true;
      CO2sum = 0.0;
      CO2counts = 0;
    }
  }
  else if(solenoid){
    digitalWrite(SOLENOID_PIN, LOW);
    solenoid = false;
  }


  // Check temperature
  checkTemp = dht.readTemperature();
  if(!isnan(checkTemp)){
    temp = checkTemp;

      // Check whether or not heater control is on 
    if(heaterStandby == false){
        if(heater){
          if(temp > (SET_TEMP-TEMP_OFFSET)){
            digitalWrite(HEATER_PIN, LOW);
            heater = false;
          }
        }
        else if(temp <= (SET_TEMP-TEMP_OFFSET)){
          digitalWrite(HEATER_PIN, HIGH);
          heater = true;
        }
    }
    else if(heater){
      digitalWrite(HEATER_PIN, LOW);
      heater = false;
    }
  }


  // Check humidity
  checkHumid = dht.readHumidity();
  if(!isnan(checkHumid)){
    humidity = checkHumid;

    // If humidity is too low, turn on the atomizer
    if(atomizerStandby == false){
      if(atomizer){
        if(humidity >= SET_HUMIDITY){
          digitalWrite(ATOMIZER_PIN, LOW);
          atomizer = false;
        }
      }
      else if(humidity < (SET_HUMIDITY-HUMID_OFFSET)){
        digitalWrite(ATOMIZER_PIN, HIGH);
        atomizer = true;
      }
      else{
        digitalWrite(ATOMIZER_PIN, LOW);
        atomizer = false;
      }
    }
    else if(atomizer){
      digitalWrite(ATOMIZER_PIN, LOW);
      atomizer = false;
    }

  }


// Output to exe
  Serial.print("p");
  Serial.println(CO2percentage);
  Serial.print("v");
  Serial.println(temp);
  Serial.print("g");
  Serial.println(humidity);


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


// Method Definitions
float MGRead(int mg_pin)
{
    int i;
    float v=0;

    for (i=0;i<READ_SAMPLE_TIMES;i++) {
        v += analogRead(mg_pin);
        delay(READ_SAMPLE_INTERVAL);
    }
    v = (v/READ_SAMPLE_TIMES) *5/1024 ;
    return v;
}

int  MGGetPercentage(float volts, float *pcurve)
{
   if ((volts/DC_GAIN) >= ZERO_POINT_VOLTAGE) {
      return -1;
   } else {
      return pow(10, ((volts/DC_GAIN)-pcurve[1])/pcurve[2]+pcurve[0]);
   }
}
