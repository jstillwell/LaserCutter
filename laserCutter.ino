/*
 * 
The MIT License (MIT)

Copyright (c) 2015 John Stillwell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 * 
 */

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
#include <SoftwareSerial.h>
#include "ESP8266.h"

#define SSID        "wifi"
#define PASSWORD    "password"

SoftwareSerial wifiSerial(8,9);
ESP8266 wifi(wifiSerial);

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *verticalMotor = AFMS.getStepper(200, 1);
Adafruit_StepperMotor *baseMotor = AFMS.getStepper(200, 2);

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps

  wifiSerial.begin(9600);
  Serial.print("Wifi setup beginning... \r\n");

  Serial.print("FW Version: ");
    Serial.println(wifi.getVersion().c_str());
    
    
    if (wifi.setOprToStation()) {
        Serial.print("to station ok\r\n");
    } else {
        Serial.print("to station err\r\n");
    }

    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");
        Serial.print("IP: ");       
        Serial.println(wifi.getLocalIP().c_str());
    } else {
        Serial.print("Join AP failure\r\n");
    }

    //setup tcp server for receiving commands
    if (wifi.enableMUX()) {
        Serial.print("multiple ok\r\n");
    } else {
        Serial.print("multiple err\r\n");
    }
    
    if (wifi.startTCPServer(8090)) {
        Serial.print("start tcp server ok\r\n");
    } else {
        Serial.print("start tcp server err\r\n");
    }
    
    if (wifi.setTCPServerTimeout(10)) { 
        Serial.print("set tcp server timout 10 seconds\r\n");
    } else {
        Serial.print("set tcp server timout err\r\n");
    }
    
    Serial.print("setup end\r\n");

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  
  verticalMotor->setSpeed(10);  // 10 rpm   
  baseMotor->setSpeed(10);
}

void loop() {
    uint8_t buffer[1024] = {0};
    uint8_t mux_id;
    uint32_t len = wifi.recv(&mux_id, buffer, sizeof(buffer), 10000);
    if (len > 0) {
        Serial.print("Status:[");
        Serial.print(wifi.getIPStatus().c_str());
        Serial.println("]");
        
        Serial.print("Received from :");
        Serial.print(mux_id);
        // Match the request
        String val;
        String req = (String)len;
        if (req.indexOf("/greeting/0") != -1){
          int strStart = req.indexOf("/greeting/0");
          int strEnd = req.indexOf("/\r/0");
          val = "greeting= " + req.substring(strStart, strEnd);
          startTest();
        }
        else {
          Serial.println("Greeting not found");
        }

//        Serial.print("[");
//        for(uint32_t i = 0; i < len; i++) {
//            Serial.print((char)buffer[i]);
//        }
//        Serial.print("]\r\n");
        
        if(wifi.send(mux_id, buffer, len)) {
            Serial.print("send back ok\r\n");
        } else {
            Serial.print("send back err\r\n");
        }
        
        if (wifi.releaseTCP(mux_id)) {
            Serial.print("release tcp ");
            Serial.print(mux_id);
            Serial.println(" ok");
        } else {
            Serial.print("release tcp");
            Serial.print(mux_id);
            Serial.println(" err");
        }
        
        Serial.print("Status:[");
        Serial.print(wifi.getIPStatus().c_str());
        Serial.println("]");
    }
  
}

void startTest(){
  Serial.println("Single coil steps");
  baseMotor->step(100, FORWARD, SINGLE); 
  baseMotor->step(100, BACKWARD, SINGLE); 

  Serial.println("Double coil steps");
  verticalMotor->step(100, FORWARD, DOUBLE); 
  baseMotor->step(100, BACKWARD, DOUBLE);
  
  Serial.println("Interleave coil steps");
  baseMotor->step(100, FORWARD, INTERLEAVE); 
  baseMotor->step(100, BACKWARD, INTERLEAVE); 
  
  Serial.println("Microstep steps");
  verticalMotor->step(50, FORWARD, MICROSTEP); 
  verticalMotor->step(50, BACKWARD, MICROSTEP);
}

