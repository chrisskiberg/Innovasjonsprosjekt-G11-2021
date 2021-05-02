/*===============KOBLINGSSKJEMA=========================
INT til pin 25
SDA til pin 21
SCL til pin 22
GND til GND
VCC til 3V3

Knapp til pin 19 
++ https://www.arduino.cc/en/Tutorial/BuiltInExamples/Button
//#include "I2Cdev.h"

*/

#include <WiFi.h>      //ESP32 Core WiFi Library    
#include <DNSServer.h>
#include <WiFiClient.h> 
#include <HTTPClient.h>


#include "MPU6050_6Axis_MotionApps20.h"
#include <Wire.h>

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector

#define INTERRUPT_PIN 25
#define BUTTON_PIN 19
//#define ERROR_PIN 13
//#define WAIT_PIN 14
#define READY_PIN 15
#define SUCCESS_PIN 16

const char DEVICE_NAME[] = "mpu6050";

String post_data = "";
bool first_connect = false;
String cookies = "";
String x_CSRFToken="";
int test=1;



//===============WIFI NAVN OG PASSORD=================== 
//const char* ssid = "Simen sin iPhone";
//const char* password = "olav1234";
const char* ssid = "Galaxy A516052";
const char* password = "mjsm4054";
int count_wifi=0;

const char * headerKeysPOST[] = {"xdata", "ydata", "zdata"};
const char * headerKeysGET[] = {"Set-Cookie"};
const size_t numberOfHeadersGET = sizeof(headerKeysGET)/sizeof(char*);
const size_t numberOfHeadersPOST = sizeof(headerKeysPOST)/sizeof(char*);

//===========LISTE FOR POST DATA===========
int x_acc[400];
int y_acc[400];
int z_acc[400];
int time_since_start[400];
String sensorData[4]; //x, y, z, tid
int start_time; // tid
int num_of_measurements = 0;
bool button_pushed = false;

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void ICACHE_RAM_ATTR dmpDataReady() {
    mpuInterrupt = true;
}

volatile bool sendPOST = false;

void mpu_setup() {
  digitalWrite(READY_PIN, HIGH);

  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin(21 , 22, 400000);
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
  #endif

  // initialize device
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();
  pinMode(INTERRUPT_PIN, INPUT);

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();
  

  // supply your own gyro offsets here, scaled for min sensitivity
  //mpu.setXGyroOffset(40);
  //mpu.setYGyroOffset(27);
  //mpu.setZGyroOffset(-22);
  //mpu.setXAccelOffset(-495);
  //mpu.setYAccelOffset(-5210);
  //mpu.setZAccelOffset(-9528); // 950
  mpu.setRate(7); // 7 (5.6) / 5 (4.6)
  digitalWrite(READY_PIN, HIGH);

//  mpu.setFullScaleGyroRange(0);
//  mpu.setFullScaleAccelRange(0);
  
  Serial.print("getFullScaleAccelRange: ");
  Serial.println(mpu.getFullScaleAccelRange());
  Serial.print("getFullScaleGyroRange: ");
  Serial.println(mpu.getFullScaleGyroRange());

  mpu.CalibrateGyro(); 
  mpu.CalibrateAccel(); 
  digitalWrite(READY_PIN, LOW);

  //  mpu.PrintActiveOffsets();
  //mpu.CalibrateGyro(); 
  //mpu.CalibrateAccel(); 
  // uint8_t MPU6050::dmpInitialize() {
  //DEBUG_PRINTLN(F("Setting sample rate to 200Hz..."));
  //setRate(4); // 1khz / (1 + 4) = 200 Hz  }

  
  // Calibration Routines
  //void CalibrateGyro(uint8_t Loops = 15); // Fine tune after setting offsets with less Loops.
  //void CalibrateAccel(uint8_t Loops = 15);// Fine tune after setting offsets with less Loops.
  //void PID(uint8_t ReadAddress, float kP,float kI, uint8_t Loops);  // Does the math
  //void PrintActiveOffsets(); // See the results of the Calibration



  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {

    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();


  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(READY_PIN, OUTPUT);
  pinMode(SUCCESS_PIN, OUTPUT);
  digitalWrite(READY_PIN, HIGH);
  digitalWrite(SUCCESS_PIN, LOW);

  Serial.print("Det funker din løk");
  Serial.println("\nOrientation Sensor real world accel output");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {  
    if (count_wifi>=5) {
      if (digitalRead(READY_PIN)) {
        digitalWrite(READY_PIN, LOW);
        digitalWrite(SUCCESS_PIN, HIGH);
      } else {
        digitalWrite(READY_PIN, HIGH);
        digitalWrite(SUCCESS_PIN, LOW);
      }
    }  
    WiFi.persistent(false);
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    //delay(3000); // Wait 3 Seconds, WL_DISCONNECTED is present until new connect!
    //vTaskDelay(250); // Check again in about 250ms

    Serial.println("Connecting to WiFi..");
    count_wifi++;
    delay(2000);
  } 
  
  digitalWrite(READY_PIN, LOW);

  mpu_setup();
  digitalWrite(READY_PIN, LOW);
  digitalWrite(SUCCESS_PIN, HIGH);
  delay(2000);
  digitalWrite(SUCCESS_PIN, LOW);
  start_time = millis();   
} 



void mpu_loop() {
  // if programming failed, don't try to do anything
  if (!dmpReady) return;

  // wait for MPU interrupt or extra packet(s) available
  if (!mpuInterrupt && fifoCount < packetSize) return;

  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;

    // display initial world-frame acceleration, adjusted to remove gravity
    // and rotated based on known orientation from quaternion
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
    mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
    //Serial.print("aworld\t");
    //Serial.print(aaWorld.x);
    //Serial.print("\t");
    //Serial.print(aaWorld.y);
    //Serial.print("\t");
    //Serial.print(aaWorld.z);
    //Serial.print("\t");
    //Serial.println(millis());
    if(button_pushed){
      Serial.print("Maaling: ");
      x_acc[num_of_measurements] = aaWorld.x;
      y_acc[num_of_measurements] = aaWorld.y;
      z_acc[num_of_measurements] = aaWorld.z;
      time_since_start[num_of_measurements] = millis()-start_time; // fjerne denne 
      num_of_measurements += 1;
      Serial.println(num_of_measurements);
     }
  //sendPOST = true; 
  }
}

 
void loop() {
 
 HTTPClient http; 
 http.collectHeaders(headerKeysGET, numberOfHeadersGET); 
 // Your Domain name with URL path or IP address with path
 http.begin("https://gruppe11.innovasjon.ed.ntnu.no/elsysapp/sensor/");


  if (!first_connect){
    digitalWrite(READY_PIN, HIGH);

    while(cookies=="") {
      if (digitalRead(READY_PIN)) {
        digitalWrite(READY_PIN, LOW);
        digitalWrite(SUCCESS_PIN, HIGH);
      } else {
        digitalWrite(READY_PIN, HIGH);
        digitalWrite(SUCCESS_PIN, LOW);
      }

    int httpCode = http.GET();
    if (httpCode == 200) { //Check for the returning code
 
        //String payload = http.getString();
        //Serial.println(httpCode);
        //Serial.println(payload);

        //=========X-CSRFTOKEN==================
        //cookies += String(http.header("Cookies"));
        
         for(int i = 0 ; i < http.headers() ; i++){
          cookies=http.header(i);
         } 
         
        Serial.print("cookies check:");
        Serial.println(cookies);
        x_CSRFToken=cookies.substring(10, 74);
        cookies=cookies.substring(0, 74);
        Serial.print("X-CSRFToken check:");
        Serial.println(x_CSRFToken);
        Serial.print("cookies check:");
        Serial.println(cookies);

        digitalWrite(READY_PIN, LOW);
        digitalWrite(SUCCESS_PIN, HIGH);
        delay(3000);
        digitalWrite(SUCCESS_PIN, LOW);       
      } else {
        Serial.println("Error on HTTP request");
        delay(1000);
      }  
      first_connect = true;
    }
   }
  //======================KJØRER MPU=====================
  if (digitalRead(BUTTON_PIN)==HIGH) {
    //test=0;
    //digitalWrite(SUCCESS_PIN, HIGH);
    delay(2000);
    //digitalWrite(SUCCESS_PIN, LOW);

    digitalWrite(READY_PIN, HIGH); // setter på LED-lys
    button_pushed=true; 
  }
  if (button_pushed==true) {

    mpu_loop();
  }

  //============HER VELGER DU ANTALL MÅLINGER
  if(num_of_measurements == 350 && !sendPOST){
    sensorData[0]="";
    sensorData[1]="";
    sensorData[2]="";    
    sensorData[3]="";
    digitalWrite(READY_PIN, LOW);
    digitalWrite(SUCCESS_PIN, HIGH);


    for(int i = 0; i < num_of_measurements-1; i++){
      sensorData[3]+=String(time_since_start[i]) + ",";
      
      if (x_acc[i]<0) {
        sensorData[0] += "-" + String((-1)*x_acc[i], HEX) + ",";
      } else {
        sensorData[0] += String(x_acc[i], HEX) +",";
      } 

      if (y_acc[i]<0) {
        sensorData[1] += "-" + String((-1)*y_acc[i], HEX) + ",";
      } else {
        sensorData[1] += String(y_acc[i], HEX) + ",";
      } 

      if (z_acc[i]<0) {
        sensorData[2] += "-" + String((-1)*z_acc[i], HEX) + ",";
      } else {
        sensorData[2] += String(z_acc[i], HEX) + ",";
      } 
      //sensorData[0] += String(x_acc[i], HEX) + ",";
      //sensorData[1] += String(y_acc[i], HEX) + ",";
      //sensorData[2] += String(z_acc[i], HEX) + ",";
      //sensorData[3] += time_since_start[i] + ","
     }
    //sensorData[0] += String(x_acc[num_of_measurements-1], HEX);
    //sensorData[1] += String(y_acc[num_of_measurements-1], HEX);
    //sensorData[2] += String(z_acc[num_of_measurements-1], HEX);
    //sensorData[3] += time_since_start[i];

    if (x_acc[num_of_measurements-1]<0) {
      sensorData[0] += "-"+ String((-1)*x_acc[num_of_measurements-1], HEX);
    } else {
      sensorData[0] += String(x_acc[num_of_measurements-1], HEX);
    } 

    if (y_acc[num_of_measurements-1]<0) {
      sensorData[1] += "-"+ String((-1)*y_acc[num_of_measurements-1], HEX);
    } else {
      sensorData[1] += String(y_acc[num_of_measurements-1], HEX);
    } 

    if (z_acc[num_of_measurements-1]<0) {
      sensorData[2] += "-"+ String((-1)*z_acc[num_of_measurements-1], HEX);
    } else {
      sensorData[2] += String(z_acc[num_of_measurements-1], HEX);
    } 


     //Fjerner siste komma i hver streng
     //sensorData[0].remove(-1);
     //sensorData[1].remove(-1);
     //sensorData[2].remove(-1);
  
     num_of_measurements=0;
     //Serial.print("DATA CHECK:");
     //Serial.print(sensorData[0]);
     //Serial.print("&&");
     //Serial.print(sensorData[1]);
     //Serial.print("&&");
     //Serial.println(sensorData[2]);
     sendPOST = true;
     button_pushed=false;
   }


  if (sendPOST){
    String postData="";
    String postTime="";

    //String headerTime=headerKeysPOST[0];
    //postTime +=headerTime + "=" + sensorData[3];
    //postTime += "&";
    //headerTime=headerKeysPOST[1];
    //postTime += headerTime + "=";
    //postTime += "&";
    //headerTime=headerKeysPOST[2];
    //postTime += headerTime + "=";
    
    //Serial.println("for-loop");            
    for ( int fieldNumber = 0; fieldNumber < numberOfHeadersPOST-1; fieldNumber++ ){
        //Serial.print("fieldNumber: ");
        //Serial.println(fieldNumber);
        String fieldName = headerKeysPOST[fieldNumber];
        String fieldData = sensorData[fieldNumber];
    //    Serial.print("fieldName: ");
      //  Serial.println(fieldName);
        //Serial.print("fieldData: ");
        //Serial.println(fieldData);
  
        postData += fieldName + "=" + fieldData;
        postData += "&";
        //Serial.println(fieldName + "=");
        //Serial.println(fieldData);

     }

      String fieldName = headerKeysPOST[numberOfHeadersPOST-1];
      String fieldData =  sensorData[numberOfHeadersPOST-1];
      //Serial.print("fieldName: ");
      //Serial.println(fieldName);
      //Serial.print("fieldData: ");
      //Serial.println(fieldData);

      postData += fieldName + "=" + fieldData;
      //Serial.println(postData);
      //Serial.println(postTime);
      //Serial.println(fieldName + "=");
      //Serial.println(fieldData);



    //=============HER POSTER VI==================
    
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-CSRFToken", x_CSRFToken); 
    http.addHeader("referer", "https://gruppe11.innovasjon.ed.ntnu.no");
    http.addHeader("Cookie", cookies);

    
    //int httpCode = http.POST(postData);
    //int httpResponseCode = http.POST(postTime);
    //Serial.print("httpResponseCode: ");  
    //Serial.println(httpResponseCode);
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {  
        if (count_wifi>=5) {
          if (digitalRead(READY_PIN)) {
            digitalWrite(READY_PIN, LOW);
            digitalWrite(SUCCESS_PIN, HIGH);
          } else {
            digitalWrite(READY_PIN, HIGH);
            digitalWrite(SUCCESS_PIN, LOW);
          }
        }  
        WiFi.persistent(false);
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        //delay(3000); // Wait 3 Seconds, WL_DISCONNECTED is present until new connect!
        //vTaskDelay(250); // Check again in about 250ms
    
        Serial.println("Connecting to WiFi..");
        count_wifi++;
        delay(1000);
      } 
    }

    int httpResponseCode = http.POST(postData);
    Serial.print("httpResponseCode: ");  
    Serial.println(httpResponseCode); 

    sendPOST = false;
    button_pushed = false;
    delay(4000);
    digitalWrite(SUCCESS_PIN, LOW);
  }
}
