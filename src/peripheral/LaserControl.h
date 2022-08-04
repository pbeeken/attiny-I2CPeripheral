/**
 * This object contains all the code needed to control the Laser pointer
 * for the Space Wall project. It is a complete refactoring from the SpaceGhost
 * project which was too slow and cumbersome for what we wanted to do.
 **/

#include <Arduino.h>

#define MILISEC 1000L
#define SEC     10000000L

uint64_t PERIODS[] = {
    64000L, 125000L, 250000L, 500000L, 750000L, 
    1000000L, 1500000L, 2000000L, 3000000L, 4000000L,
};

/**
 * LaserControl contains all the operation and management of the LaserPointer
 * The device is powered directly from a PWM port on an ATTiny that is acting
 * as an I2C 'Peripheral' on a bus along with two servo's that aim the pointer at
 * the solarsystem wall. 
 **/
class LaserControl {
    public:
        volatile union {
            uint8_t raw[4];         // raw bytes
            struct {
                uint8_t reset       : 3; // if set to 7 reset
                uint8_t             : 0; // restart to next boundary
                uint8_t onOff       : 1; // 0-off 1-on
                uint8_t             : 0; // restart to next boundary
                uint8_t intensity   : 8; // current set intensity
                uint8_t period      : 4; // period index
                uint8_t mode        : 2; // mode
                uint8_t             : 2; // unused
            } data;
            uint32_t zero;          // easy reset one and done.
        } registers;

        uint8_t  laserPin;
        bool     laserOn;           // Used for quick on and off
        int16_t  curIntensity;      // for modes when it varies, this is the brightness that is used to light the 
        uint32_t actPeriod;            // actual period (when varying)
        uint32_t nextEvent;         // time to do something
        int16_t  stepIntensity;     // step to change brightness in PULSE

        enum {  // Modes
            STEADY=0, BLINK=1, PULSE=2
        };

    /**
     * Consrtuctor and initializes variables. Only required value is pin which defines
     * the pin to which the laser is connected.
     **/
    LaserControl(uint8_t pin) {
        laserPin = pin;
        reset();
    }

    /**
     * @brief Used to reset the chip to startup state
     * 
     */
    void reset() {
        registers.zero = 0x06000000;  // zero everything set period to 6th index
        curIntensity = 0;
        nextEvent = micros();
        actPeriod = PERIODS[registers.data.period]; 
        registers.data.onOff = 1;
    }

    /**
     * @brief Set the I2C 'register'
     * 
     * @param i index 
     * @param v value
     */
    void setRegister(uint8_t i, uint8_t v) {
        i = constrain(i, 0, sizeof(registers.raw) / sizeof(registers.raw[0]));
        registers.raw[i] = v;
    }

    /**
     * @brief Get the I2C 'register'
     * 
     * @param i index 
     * @return int8_t value 
     */
    int8_t getRegister(uint8_t i) {
        i = constrain(i, 0, sizeof(registers.raw) / sizeof(registers.raw[0]));
        return registers.raw[i];
    }

    /**
     * @brief Called at 'setup()' to start things rolling. 
     *        Traditional way to kick off peripherals on arduinos
     */
    void begin() {
        pinMode(laserPin, OUTPUT); // OC1A, also The only HW-PWM -pin supported by the tiny core analogWrite
        reset();
    }

    /**
     * @brief Set the maximum brightness level
     */
    void setIntensity(int16_t brightness) {
        registers.data.intensity = constrain(brightness, 0, 255);
        curIntensity = registers.data.intensity;
        nextEvent = micros();
    }

    /**
     * @brief Get the current brightness level
     */
    uint8_t getIntensity() {
        return (uint8_t)0xFF & curIntensity;
    }

    /**
     * @brief Set the mode: Steady, Blink, and Pulse
     */
    void setMode(int16_t mode) {
        registers.data.mode = mode;
        if (mode==STEADY) {
            actPeriod = PERIODS[registers.data.period];  // reset period
            nextEvent = micros();
            }
        else if (mode==BLINK) {
            stepIntensity = 0;  // doesn't matter because we don't do this.
            nextEvent = micros() + (actPeriod/4);
            curIntensity = 0; // start off
            }
        else if (mode==PULSE) {
            stepIntensity = registers.data.intensity / 16;  // 16 levels
            nextEvent = micros() + (actPeriod/4)/16;
            curIntensity = 0; // start off
            }
        }

    /**
     * @brief Get the mode: Steady, Blink, and Pulse
     */
    uint8_t getMode() {
        return registers.data.mode;
        }

    /**
     * @brief Set the period in 16msec intervals
     */
    void setPeriod(uint8_t per) {
        registers.data.period = constrain(per, 0, sizeof(PERIODS)/sizeof(PERIODS[0]));
        actPeriod = PERIODS[per];   
        if (getMode()==BLINK)
            nextEvent = micros() + (actPeriod/4);
        else if (getMode()==PULSE)
            nextEvent = micros() + (actPeriod/4)/16;
    }

    /**
     * @brief Get the period in 16msec intervals
     */
    int getPeriod() {
        for(uint8_t i=0; i<sizeof(PERIODS)/sizeof(PERIODS[0]); i++)
            if (actPeriod == PERIODS[i]) return i;
        return 0;
    }


    /**
     * @brief update the laser output. Called in the 'loop()'
     */
    void update() {
        if (registers.data.onOff==0) {
            // if quick off is active then just turn off the laser
            analogWrite(laserPin, 0);
            return;  // we are done.
            }

        if (registers.data.mode == STEADY) { 
            // don't do anything just make sure curInensity is up to date.
            curIntensity = registers.data.intensity;
            }

        else if (registers.data.mode == BLINK && micros() > nextEvent) {
            if (curIntensity == 0) curIntensity = registers.data.intensity;
            else                   curIntensity = 0;
                nextEvent = micros() + (actPeriod/4);
            }

        else if (registers.data.mode == PULSE && micros() > nextEvent) {
            curIntensity += stepIntensity;
            if (curIntensity < 0)  {
                curIntensity = 0; 
                stepIntensity = -stepIntensity;
            }
            else if (curIntensity > registers.data.intensity) {
                curIntensity = registers.data.intensity; 
                stepIntensity = -stepIntensity;
            }
            nextEvent = micros() + (actPeriod/4)/16;
        }

    // The core operation for this "peripheral"
    analogWrite(laserPin, curIntensity);        // update pin.
    }
};