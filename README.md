# attiny-I2CPeripheral

Although this project has a specific purpose it can serve as a model for other I2C 
implementations as this one will entertain many special details with timing.

The idea is to make the Robosapien (RS) controller an I2C device that accepts the 1 byte commnands
to control the robot. This allows 3.3V as well as TTL level controls which allows us to 
replace the the primary processor.  Right out of the box there will be challenges.

The ATTiny implementation I will be using is Rambo's TinyWireS library which already comes
with an issue using `delay()` and `microsecondDelay()`$^\dagger$ as this upsets the timing needed to 
pay attention to the I2C bus.  I use these tools to control the signal toggling needed to mimic
the IR codes that control the RS.  Fortunately there are a couple of 
[potential workarounds](https://github.com/rambo/TinyWire/issues/8) and the 
empirical fact that the timing needed to control the RS is not terribly sensitive. Once
this is implemented we'll see what we can get away with.

This is a good model for generalizing the I2C controller. It sets up multi-byte commands and returns.  
More research revealed that I was working harder than I had to on the ATTiny,

  1. Set the FUSEs for 8MHz.
  2. Decide on an I2C address for the "slave"
  3. Decide on Command/Response protocols for communications.

Debugging is tricky but you can return state information in a response.  Visual feedback (bi-stable LED) is helpful.

Latest version works to control brightness, alternate from Steady, Blink, and Pulse control.  

I have implemented varying instruction length with success but what seems to work most reliably is keeping your 
instruction protocol simple and straightforward.  

## I want to drive a laser pointer.

Commands take the form of 2 characters.
  - character 1 is **I**ntensity, **M**ode, **P**, period
  - character 2 is a number or letter depending on the command character (1)
     - if the $1^{st}$ is **I** then the $2^{nd}$ is a value from 0 $\rightarrow$ 255
     - if the $1^{st}$ is **M** then the $2^{nd}$ is a character **S**teady, **B**link, or **P**ulse (heartbeat)
     - if the $1^{st}$ is **P** then the $2^{nd}$ is a string from '0' $\rightarrow$ '9' which is index into the following intervals.

 |char|period in ms|
 |:-:|--:|
 |0|64|
 |1|125|
 |2|250|
 |3|500|
 |4|750|
 |5|1000|
 |6|1500|
 |7|2000|
 |8|3000|
 |9|4000|


$^\dagger$ This is a general issue when you take charge of interrupts on the ATTiny. You need to rethink how you do timing.
