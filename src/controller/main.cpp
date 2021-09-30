#include <Arduino.h>

#define I2C_SLAVE_ADDRESS 0x14 // the 7-bit address (remember to change this when adapting this example)
#include <Wire.h>
#include <Streaming.h>


void setup() {
    Wire.begin();
    Wire.setWireTimeout(2000);
    Serial.begin(115200);
}


void loop() {
    if (Serial.available()>0) {
        char cc = Serial.read();
        if (cc=='R') {
            Wire.requestFrom(I2C_SLAVE_ADDRESS, 1);
            char c1 = Wire.read(); // first character
            char c2 = '-'; //Wire.read(); // second character
            Serial << cc << "Response [" << c1 << c2 << "]" << endl;
        }
        else if (isAlphaNumeric(cc)) {
            Wire.beginTransmission(I2C_SLAVE_ADDRESS);
            Wire.write(cc);
            //Wire.write('O');
            Wire.endTransmission();
            Serial << cc << "Transmit" << endl;
        }
    }
    delay(500);
}