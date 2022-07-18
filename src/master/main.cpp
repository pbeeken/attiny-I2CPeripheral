#include <Arduino.h>

#define I2C_SLAVE_ADDRESS 0x14 // the 7-bit address (remember to change this when adapting this example)
#include <Wire.h>
#include <Streaming.h>


void setup() {
    Wire.begin();
    Wire.setWireTimeout(2000);
    Serial.begin(115200);
}

uint8_t Blinker = 0;

void loop() {
    if (Serial.available()>0) {
        char cc = Serial.read();
        if (cc=='R') {
            uint8_t buff[2];
            Wire.requestFrom(I2C_SLAVE_ADDRESS, 2);
            buff[0] = (uint16_t) Wire.read();
            buff[1] = Wire.read();
            Serial << cc << "esponse [" << _DEC(buff[0]) << "-" << buff[1] << "] " << endl;
        }
        else if (cc=='B') {
            uint8_t buff[2];
            buff[0] = 'M';
            buff[1] = 'B';
            Wire.beginTransmission(I2C_SLAVE_ADDRESS);
            Wire.write(buff, 2);
            Wire.endTransmission();
            Serial << cc << "Transmit" << endl;        
        }
        else if (cc=='S') {
            uint8_t buff[2];
            buff[0] = 'M';
            buff[1] = 'S';
            Wire.beginTransmission(I2C_SLAVE_ADDRESS);
            Wire.write(buff, 2);
            Wire.endTransmission();
            Serial << cc << "Transmit" << endl;        
        }
        else if (cc=='P') {
            uint8_t buff[2];
            buff[0] = 'M';
            buff[1] = 'P';
            Wire.beginTransmission(I2C_SLAVE_ADDRESS);
            Wire.write(buff, 2);
            Wire.endTransmission();
            Serial << cc << "Transmit" << endl;        
        }        
        else if (isDigit(cc)) {
            uint8_t buff[2];
            buff[0] = 'I';
            buff[1] = ((uint8_t)cc-'0')*20;
            Wire.beginTransmission(I2C_SLAVE_ADDRESS);
            Wire.write(buff, 2);
            Wire.endTransmission();
            Serial << cc << "[" << buff[1] << "]" << "Transmit" << endl;
        }
    }
    delay(100);
}