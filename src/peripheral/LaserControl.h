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
        uint8_t  laserPin;
        int16_t  maxBrightness;      // value between 0(off) to 255(full on)
        int16_t  curBrightness;      // for modes when it varies, this is the current brightness
        uint8_t  curMode;            // current mode 
        uint32_t period;             // period when varying
        uint32_t nextEvent;          // time to do something
        int16_t  step;               // step to change brightness in PULSE

        enum {  // Modes
            STEADY='S', BLINK='B', PULSE='P'
        };

    /**
     * Consrtuctor and initializes variables. Only required value is pin which defines
     * the pin to which the laser is connected.
     **/
    LaserControl(uint8_t pin, int16_t brightness=0, uint8_t mode=STEADY) {
        laserPin = pin;
        maxBrightness = brightness;
        curBrightness = maxBrightness;
        nextEvent = micros();
        period = 500*MILISEC; // 0.5 second
        curMode = mode;
    }

    /**
     * @brief Called at 'setup()' to start things rolling. 
     *        Traditional way to kick off peripherals on arduinos
     */
    void begin() {
        pinMode(laserPin, OUTPUT); // OC1A, also The only HW-PWM -pin supported by the tiny core analogWrite
    }

    /**
     * @brief Set the maximum brightness level
     */
    void setBrightness(int16_t brightness) {
        maxBrightness = brightness;
        curBrightness = brightness;
    }

    /**
     * @brief Get the current brightness level
     */
    uint8_t getBrightness() {return (uint8_t)0xFF & curBrightness;}

    /**
     * @brief Set the mode: Steady, Blink, and Pulse
     */
    void setMode(int16_t mode) {
        curMode = mode;
        if (curMode==STEADY) {
            period = 500*MILISEC;  // reset period
            nextEvent = micros();
            }
        else if (curMode==BLINK) {
            step = 0;  // doesn't matter because we don't do this.
            nextEvent = micros() + (period/4);
            curBrightness = 0; // start off
            }
        else if (curMode==PULSE) {
            step = maxBrightness / 16;  // 16 levels
            nextEvent = micros() + (period/4)/16;
            curBrightness = 0; // start off
            }
        }

    /**
     * @brief Get the mode: Steady, Blink, and Pulse
     */
    uint8_t getMode() {return curMode;}

    /**
     * @brief Set the period in 16msec intervals
     */
    void setPeriod(int per) {
        per = constrain(per, 0, 9);
        period = PERIODS[per];        
        if (curMode==BLINK)
            nextEvent = micros() + (period/4);
        else if (curMode==PULSE)
            nextEvent = micros() + (period/4)/16;
    }

    /**
     * @brief Get the period in 16msec intervals
     */
    int getPeriod() {
        for(int i=0; i<10; i++)
            if (period == PERIODS[i]) return i;
        return -1;
    }


    /**
     * @brief update the laser output. Called in the 'loop()'
     */
    void update() {
        if (curMode == STEADY) { 
            // don't do anything
            curBrightness = maxBrightness;
            }

        else if (curMode == BLINK && micros() > nextEvent) {
            if (curBrightness == 0) curBrightness = maxBrightness;
            else                    curBrightness = 0;
                nextEvent = micros() + (period >> 1);
            }

        else if (curMode == PULSE && micros() > nextEvent) {
            curBrightness += step;
            if (curBrightness < 0)             {curBrightness = 0; step = -step;}
            if (curBrightness > maxBrightness) {curBrightness = maxBrightness; step = -step;}
            nextEvent = micros() + (period >> 1)/16;
        }

    // The core operation for this "peripheral"
    analogWrite(laserPin, curBrightness);        // update pin.
    }
};