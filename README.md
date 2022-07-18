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


$^\dagger$ This is a general issue when you take charge of interrupts on the ATTiny. You need to rethink how you do timing.

Latest version works to control brightness, alternate from Steady, Blink, and Pulse control.  