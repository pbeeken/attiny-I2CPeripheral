#include <Arduino.h>

#define I2C_SLAVE_ADDRESS 0x14 // the 7-bit address (remember to change this when adapting this example)
#include <TinyWireS.h>

// The default buffer size, Can't recall the scope of defines right now
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif

// Declarations
void requestEvent();
void receiveEvent(uint8_t howMany);

volatile uint8_t i2c_regs[] = {'O', 'K',};
volatile uint8_t cc = '!';
// Tracks the current register pointer position
volatile byte reg_position = 0;
const byte reg_size = sizeof(i2c_regs);

void blink(uint8_t color, uint8_t blinks=2);
const uint8_t RED = 0x34;
const uint8_t GRN = 0x43;


void setup() {
    pinMode(3, OUTPUT); // OC1B-, Arduino pin 3, ADC
    pinMode(4, OUTPUT); // OC1B-, Arduino pin 3, ADC
    digitalWrite(3, LOW); // Note that this makes the led turn on, it's wire this way to allow for the voltage sensing above.
    digitalWrite(4, LOW); // Note that this makes the led turn on, it's wire this way to allow for the voltage sensing above.

    //pinMode(1, OUTPUT); // OC1A, also The only HW-PWM -pin supported by the tiny core analogWrite

    /**
     * Reminder: taking care of pull-ups is the masters job
     */

    TinyWireS.begin(I2C_SLAVE_ADDRESS);
    TinyWireS.onReceive(receiveEvent);
    TinyWireS.onRequest(requestEvent);

    
    // Whatever other setup routines ?
    blink(RED, 4);
    blink(GRN, 4);
}

const int IDLE = 0;
const int WROTE = 1;
const int READ = 2;

volatile int state = IDLE;

void loop() {
    /**
     * This is the only way we can detect stop condition (http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=984716&sid=82e9dc7299a8243b86cf7969dd41b5b5#984716)
     * it needs to be called in a very tight loop in order not to miss any (REMINDER: Do *not* use delay() anywhere, use tws_delay() instead).
     * It will call the function registered via TinyWireS.onReceive(); if there is data in the buffer on stop.
     */
    TinyWireS_stop_check();

    if (state == WROTE) {
        blink(RED);
        state = IDLE;
    } else if (state == READ) {
        blink(GRN);
        state = IDLE;
    }

}



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
 * This is called for each read request we receive, never put more than one byte of data (with TinyWireS.send) 
 * to the send-buffer when using this callback
 */
void requestEvent() { 
    TinyWireS.send(cc);
    // Increment the reg position on each read, and loop back to zero
    // reg_position++;
    // if (reg_position >= reg_size) {
    //     reg_position = 0;
    // }
    state = WROTE;
}

/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, data, restart/stop) on the bus does
 * so be quick, set flags for long running tasks to be called from the mainloop instead of running them directly,
 */
void receiveEvent(uint8_t howMany) {
    if (howMany < 1 || howMany > TWI_RX_BUFFER_SIZE) {
        // Sanity-check
        return;
    }

    cc = TinyWireS.receive();
    // howMany--;
    // if (howMany==0) {
    //     // This write was only to set the buffer for next read
    //     return;
    // }

    // while(howMany--) {
    //     cc = TinyWireS.recieve();
    //     // i2c_regs[reg_position] = TinyWireS.receive();
    //     // reg_position++;
    //     // if (reg_position >= reg_size) {
    //     //     reg_position = 0;
    //     // }
    // }
    state = READ;
}
