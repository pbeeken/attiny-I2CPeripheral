#include <Arduino.h>

#define I2C_SLAVE_ADDRESS 0x14 // the 7-bit address (remember to change this when adapting this example)
#include <Wire.h>
#include <Streaming.h>


void setup() {
    Wire.begin();
    Wire.setWireTimeout(2000);
    Serial.begin(115200);
}

#define _CHAR(b) ((char)b)

uint8_t Blinker = 0;

void loop() {
    uint8_t buff[4];

    if (Serial.available()>0) {
        char cc = Serial.read();
        if (cc=='R') {
            Wire.requestFrom(I2C_SLAVE_ADDRESS, 2);
            buff[0] = (uint16_t) Wire.read();
            buff[1] = Wire.read();
            Serial << "Response [" << _DEC(buff[0]) << "-" << _CHAR(buff[1]) << "] " << endl;
        }
        else if (cc=='B') {
            buff[0] = 'M';
            buff[1] = 'B';
            Wire.beginTransmission(I2C_SLAVE_ADDRESS);
            Wire.write(buff, 2);
            Wire.endTransmission();
            Serial << "Transmit " << "[" << _CHAR(buff[0]) << _CHAR(buff[1]) << "]" << endl;
        }
        else if (cc=='b') {
            buff[0] = 'M';
            buff[1] = 'B';
            buff[2] = 16;
            Wire.beginTransmission(I2C_SLAVE_ADDRESS);
            Wire.write(buff, 3);
            Wire.endTransmission();
            Serial << "Transmit " << "[" << _CHAR(buff[0]) << _CHAR(buff[1]) << _DEC(buff[2]) << "]" << endl;
        }
        else if (cc=='S') {
            buff[0] = 'M';
            buff[1] = 'S';
            Wire.beginTransmission(I2C_SLAVE_ADDRESS);
            Wire.write(buff, 2);
            Wire.endTransmission();
            Serial << "Transmit " << "[" << _CHAR(buff[0]) << _CHAR(buff[1]) << "]" << endl;
        }
        else if (cc=='P') {
            buff[0] = 'M';
            buff[1] = 'P';
            Wire.beginTransmission(I2C_SLAVE_ADDRESS);
            Wire.write(buff, 2);
            Wire.endTransmission();
            Serial << "Transmit " << "[" << _CHAR(buff[0]) << _CHAR(buff[1]) << "]" << endl;
        }        
         else if (cc=='p') {
            buff[0] = 'M';
            buff[1] = 'P';
            buff[2] = 64;
            Wire.beginTransmission(I2C_SLAVE_ADDRESS);
            Wire.write(buff, 3);
            Wire.endTransmission();
            Serial << "Transmit " << "[" << _CHAR(buff[0]) << _CHAR(buff[1]) << _DEC(buff[2]) << "]" << endl;
        }        
       else if (isDigit(cc)) {
            buff[0] = 'I';
            buff[1] = ((uint8_t)cc-'0')*20;
            Wire.beginTransmission(I2C_SLAVE_ADDRESS);
            Wire.write(buff, 2);
            Wire.endTransmission();
            Serial << "Transmit " << "[" << _CHAR(buff[0]) << cc << "]" << endl;
        }
    }
    delay(100);
}