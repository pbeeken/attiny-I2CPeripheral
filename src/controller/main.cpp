#include <Arduino.h>

#define I2C_PERIPHERAL_ADDRESS 0x14 // the 7-bit address (remember to change this when adapting this example)
#include <Wire.h>
#include <Streaming.h>


void setup() {
    Wire.begin();
    Wire.setWireTimeout(2000);
    Serial.begin(115200);
}

#define _CHAR(b) ((char)b)

uint8_t Blinker = 0;
uint8_t intens = 0;
uint8_t mode = 0;
uint8_t period = 0;
uint8_t onoff = 0;
uint8_t iad = 1;

void GetRegister(uint8_t internalAddress) {            
    uint8_t val, rc=0;

    while(Wire.available()) val = Wire.read(); // empty any buffer

    Wire.beginTransmission(I2C_PERIPHERAL_ADDRESS);
    Wire.write(internalAddress);
    Wire.endTransmission();
    rc = Wire.requestFrom(I2C_PERIPHERAL_ADDRESS, 1); // This register is 8 bits = 1 byte long
    
    uint32_t timeout = micros() + 5000;
    while( !Wire.available() && timeout>micros() );
    
    val = (uint8_t)Wire.read();
    Serial << "State (" << rc << ")" << "[" << internalAddress  <<"]: " << val << ", 0x" << _HEX(val) << ", 0b" << _BIN(val) << endl;
}


void loop() {
    if (Serial.available()>0) {
        char cc = Serial.read();
        if (cc=='R') {
            Wire.beginTransmission(I2C_PERIPHERAL_ADDRESS);
            Wire.write(0);
            Wire.write(7);
            Wire.endTransmission();
            Serial << "Reset" << endl;
        }
        else if (cc=='I') {
            Wire.beginTransmission(I2C_PERIPHERAL_ADDRESS);
            Wire.write(2);
            Wire.write(intens);
            Wire.endTransmission();
            Serial << "Transmit " << "[" << _DEC(intens) << " 0x" << _HEX(intens) << "]" << endl;
            intens += 10;
            intens %= 200;
        }
        else if (cc=='M') {
            Wire.beginTransmission(I2C_PERIPHERAL_ADDRESS);
            Wire.write(3);
            Wire.write((mode<<4) | period);
            Wire.endTransmission();
            Serial << "Transmit " << "[" << mode << "-" << period << "] 0x" << _HEX((mode<<4) | period) << endl;
        }
        else if (cc=='m') {
            mode += 1;
            mode %= 3;
            Serial << "Changed mode: " << mode << endl;
        }
        else if (cc=='P') {
            Wire.beginTransmission(I2C_PERIPHERAL_ADDRESS);
            Wire.write(3);
            Wire.write((mode<<4) | period);
            Wire.endTransmission();
            Serial << "Transmit " << "[" << mode << "-" << period << "] 0x" << _HEX((mode<<4) | period) << endl;
        }
        else if (cc=='p') {
            period += 1;
            period %= 10;
            Serial << "Changed period: " << period << endl;
        }
        else if (cc=='T') {
            Wire.beginTransmission(I2C_PERIPHERAL_ADDRESS);
            Wire.write(1);
            Wire.write(onoff);
            Wire.endTransmission();
            Serial << "State " << "[" << onoff << "]" << endl;
            onoff += 1;
            onoff %= 2;
        }
        else if (cc=='1') {
            GetRegister(1);
        }
        else if (cc=='2') {
            GetRegister(2);
        }
        else if (cc=='3') {
            GetRegister(3);
        }
        else if (cc=='s') {
            iad++;
            iad = 1 + (iad%3);
            Serial << " internal register [" << iad << "]" << endl;
        }
    }
  delay(100);
}