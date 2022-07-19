/**
 * @file main.cpp
 * @author Paul Beeken (pbeeken@byramhills.net)
 * @brief Example for using the ATTiny85 as an I2C subordinate
 * @version 0.8
 * @date 2021-07-15 initial
 *
 * This is meant to be used as a template for the 'firmware' on an ATTiny85 to create
 * the 'Slave' for a 'Master' in order to offload tasks for communicatung or
 * controlling some electronic device.  This particular example is used to control the
 * brightness of a laser pointer.  It also provides for modulating the light 
 * (continuous, pulsed, etc). The ultimate goal is to create a display indicator.
 * Debugging the slave is tricky even when you have an oscilloscope as you need to know
 * when the device has successfully interpreted a command. In this template I use a bicolored
 * LED which will light red/green depending on the polarity of pins not used for something else.
 * You can use this as a visual cue as to whether the commands sent are interpretted the way
 * you expect.
 */

#include <Arduino.h>

// The I2C address on the bus. This value needs to be unique w.r.t. other devices on the bus
#define I2C_SLAVE_ADDRESS 0x14 // the 7-bit address (remember to change this when adapting this example)
#include <TinyWireS.h>
#include "LaserControl.h"

// The default buffer size, Can't recall the scope of defines right now
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif

// Declarations
/**
 * Method that handles a response for information.  The specifics of what is being asked for
 * would be sent in a previous receiveEvent which would set the state of this peripheral.
 **/
void requestEvent();
/**
 * Method that handles revceiving data from the "master". The information here would set the
 * the state of the machine.
 **/
void receiveEvent(uint8_t howMany);

/**
 * completed states
 **/
enum {
    IDLE, WROTE, READ,  // Operational states
};

// global scopes
uint8_t State = IDLE;
const uint8_t LASER_PIN = PB1; 
LaserControl laser(LASER_PIN);

/**
 * @brief This is the buffer that holds the incoming data.
 *        note the data type. Many issues that arose in past debugging
 *        were resolved by being strict with data types. 
 */
volatile uint8_t i2c_rcvbuffer[5];

// Forward declarations for routines:
void blink(uint8_t color, uint8_t blinks=2);
const uint8_t RED = 0x34; // Using bidirectional LED for debugging
const uint8_t GRN = 0x43; // Not required for final version.

/**
 * @brief The setup method.
 * 
 */
void setup() {
    // debugging LED
    pinMode(3, OUTPUT); // OC1B-, Arduino pin 3, ADC
    pinMode(4, OUTPUT); // OC1B-, Arduino pin 4, ADC
    digitalWrite(3, LOW); // Note that this makes the led turn on, it's wire this way to allow for the voltage sensing above.
    digitalWrite(4, LOW); // Note that this makes the led turn on, it's wire this way to allow for the voltage sensing above.

    laser.begin();

    /**
     * Reminder: taking care of pull-ups is the master's (controller's) job
     */
    TinyWireS.begin(I2C_SLAVE_ADDRESS);
    TinyWireS.onReceive(receiveEvent);
    TinyWireS.onRequest(requestEvent);

    /**
     *  Flag the debugging LED
     **/
    blink(RED, 4);
    blink(GRN, 4);
}

/**
 * @brief The main event loop.
 * 
 */
void loop() {
    /**
     * This is the only way we can detect stop condition (http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=984716&sid=82e9dc7299a8243b86cf7969dd41b5b5#984716)
     * it needs to be called in a very tight loop in order not to miss any (REMINDER: Do *not* use delay() anywhere, use tws_delay() instead).
     * It will call the function registered via TinyWireS.onReceive(); if there is data in the buffer on stop.
     */
    TinyWireS_stop_check();
    // Act on the current state.

    // We just responded to a data request.
    if (State == WROTE) {
        blink(RED, 2);
        State = IDLE;

    // we just responded to a command, the state of our peripheral may have changed.
    } else if (State == READ) {
        blink(GRN, 2);

        if (i2c_rcvbuffer[0]=='I') {
            laser.setBrightness(i2c_rcvbuffer[1]);
        }

        if (i2c_rcvbuffer[0]=='M') {
            laser.setMode(i2c_rcvbuffer[1]);  // mode character    
        }

        if (i2c_rcvbuffer[0]=='P') {
            laser.setPeriod(i2c_rcvbuffer[1]-'0');  // index into periods    
        }

        laser.update();
        State = IDLE;
    
    // we are just waiting.  This is where you might put periodic tasks.
    } else if (State == IDLE ) {
        laser.update();
    }

}

/**
 * @brief Fires the signal bipolar LED. This operation isn't strictly necessary but
 *          is useful for debugging.
 * 
 * @param color red or green (or whatever two colors)
 * @param blinks how many blinks to do (count)
 */
void blink(uint8_t color, uint8_t blinks) {
    while(blinks--) {
        digitalWrite(color & 0x0F, LOW);
        tws_delay(50);
        digitalWrite(color & 0x0F, HIGH);
        tws_delay(100);
    }
    digitalWrite(color & 0x0F, LOW);
    digitalWrite((color & 0xF0) >> 4, LOW);
}

/**
 * This is called for each read request we receive, never put more than one byte of 
 * data (with TinyWireS.send) at a time so we transmit them with individual calls
 * (contrast this with the send command in Wire.send) to the send-buffer when using 
 * this callback is installed as an ISR
 */
void requestEvent() {
    TinyWireS.send(laser.getBrightness());
    TinyWireS.send(laser.getMode());
    State = WROTE;
}

/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, data, restart/stop) 
 * on the bus does so be quick, set flags for long running tasks to be called from the 
 * mainloop instead of running them directly, So the strategy is to set state values and 
 * act on them during idle.
 * @param howMany should be the number of bytes.  In practice we ignore this so we just 
 * operate on the expected # of values.
 */
void receiveEvent(uint8_t howMany) {
    uint8_t i = 0;
    // In the current model we are expecting 2 bytes.
    while (1 < TinyWireS.available()) { // loop through all but the last
        i2c_rcvbuffer[i++] = TinyWireS.receive();      // receive byte as a character
        }
    i2c_rcvbuffer[i++] = TinyWireS.receive();    // receive byte as an integer
    i2c_rcvbuffer[i] = 0;                        // put default 0 as last value
    State = READ;
}
