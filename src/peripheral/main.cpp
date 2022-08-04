/**
 * @file main.cpp
 * @author Paul Beeken (pbeeken@byramhills.net)
 * @brief Example for using the ATTiny85 as an I2C subordinate
 * @version 0.8
 * @date 2021-07-15 initial
 *
 * This is meant to be used as a template for the 'firmware' on an ATTiny85 to create
 * the 'Peripheral' for a 'Controller' in order to offload tasks for communicatung or
 * controlling some electronic device.  This particular example is used to control the
 * brightness of a laser pointer.  It also provides for modulating the light 
 * (continuous, pulsed, etc). The ultimate goal is to create a display indicator.
 * Debugging the Peripheral is tricky even when you have an oscilloscope as you need to know
 * when the device has successfully interpreted a command. In this template I use a bicolored
 * LED which will light red/green depending on the polarity of pins not used for something else.
 * You can use this as a visual cue as to whether the commands sent are interpretted the way
 * you expect.
 */

#include <Arduino.h>
#include <TinyWireS.h>

// The I2C address on the bus. This value needs to be unique w.r.t. other devices on the bus
#include "LaserControl.h"
#define I2C_Peripheral_ADDRESS 0x14 // the 7-bit address (remember to change this when adapting this example)


// The default buffer size, Can't recall the scope of defines right now
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif

#define DEBUG

// Declarations
/**
 * Method that handles a response for information.  The specifics of what is being asked for
 * would be sent in a previous receiveEvent which would set the state of this peripheral.
 **/
void requestEvent();
/**
 * Method that handles revceiving data from the "Controller". The information here would set the
 * the state of the machine.
 **/
void receiveEvent(uint8_t howMany);

/**
 * completed states
 **/
enum {
    IDLE, GET_REG, SET_REG,  // Operational states
};

// global scopes
uint8_t State = IDLE;

// Device specific values
const uint8_t LASER_PIN = PB1; 
LaserControl laser(LASER_PIN);
uint8_t DEVICE_REGISTER = 0; // device register requested for read or write.

/**
 * @brief This is the buffer that holds the incoming data.
 *        note the data type. Many issues that arose in past debugging
 *        were resolved by being strict with data types. 
 */
volatile uint8_t i2c_register;
volatile uint8_t i2c_rcvbuffer[4];
volatile uint8_t i2c_payload; 

/**
 * @brief This is the internal pseudo register which controls the state 
 *        of our machine.  For the Laser we only need 4
 *        0: Reset command (when = 7 we do a reset)
 *        1: On/Off 
 *        2: Intensity value
 *        3: mode / freq 
 */
// volatile uint8_t registers[5]; We are keeping this in the Laser class

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
#ifdef DEBUG    
    pinMode(PB3, OUTPUT); // OC1B-, Arduino pin 3, ADC
    pinMode(PB4, OUTPUT); // OC1B-, Arduino pin 4, ADC
    digitalWrite(PB3, LOW); // Note that this makes the led turn on, it's wire this way to allow for the voltage sensing above.
    digitalWrite(PB4, LOW); // Note that this makes the led turn on, it's wire this way to allow for the voltage sensing above.
#endif
    laser.begin();

    /**
     * Reminder: taking care of pull-ups is the Controller's (controller's) job
     */
    TinyWireS.begin(I2C_Peripheral_ADDRESS);
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
    // TODO: We aren't reading the intenal registers.
    if (State == GET_REG) {
        blink(RED, 2);
        i2c_payload = laser.registers.raw[i2c_register];
        State = IDLE;

    // we just responded to a command, the state of our peripheral may have changed.
    } else if (State == SET_REG) {
        blink(GRN, 2);
        // one exception, i2c_rcvbuffer[0] == 7 and i2c_register == 0
        if ((i2c_register == 0) && (i2c_rcvbuffer[0]==7)) {
            laser.reset();
            blink(RED,8);
        }
        else {
            laser.registers.raw[i2c_register] = i2c_rcvbuffer[0];
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
#ifdef DEBUG    
    while(blinks--) {
        digitalWrite(color & 0x0F, LOW);
        tws_delay(50);
        digitalWrite(color & 0x0F, HIGH);
        tws_delay(100);
    }
    digitalWrite(color & 0x0F, LOW);
    digitalWrite((color & 0xF0) >> 4, LOW);
#endif
}

/**
 * This is called for each read request we receive, never put more than one byte of 
 * data (with TinyWireS.send) at a time so we transmit them with individual calls
 * (contrast this with the send command in Wire.send) to the send-buffer when using 
 * this callback is installed as an ISR
 */
void requestEvent() {
    TinyWireS.send(i2c_payload);
    State = IDLE;
}

/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, data, restart/stop) 
 * on the bus does so be quick, set flags for long running tasks to be called from the 
 * mainloop instead of running them directly, So the strategy is to set state values and 
 * act on them during idle.
 * @param howMany should be the number of bytes.  
 */
void receiveEvent(uint8_t howMany) {
    if (howMany < 1) return;                    // Sanity-check
    if (howMany > TWI_RX_BUFFER_SIZE) return;   // Also insane number

    i2c_register = TinyWireS.receive();         // Store the register
    howMany--;
    if (howMany==0) { // This write was only to set the buffer for next read
        State = GET_REG;
        return;
    }
    uint8_t i = 0;
    while (howMany--) { // loop through all but the last
        i2c_rcvbuffer[i++] = TinyWireS.receive();      // receive byte as a character
        }
    i2c_rcvbuffer[i] = 0;                        // put default 0 as last value
    State = SET_REG;
}
