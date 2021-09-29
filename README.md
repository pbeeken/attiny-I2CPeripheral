# attiny-I2CSlave
# I2C Slave explorations for RoboSapien

Although this project has a specific purpose it can serve as a model for other I2C 
implementations as this one will entertain many special details with timing.

The idea is to make the Robosapien (RS) controller an I2C device that accepts the 1 byte commnands
to control the robot. This allows 3.3V as well as TTL level controls which allows us to 
replace the the primary processor.  Right out of the box there will be challenges.

The ATTiny implementation I will be using is Rambo's TinyWire library which already comes
with an issue using `delay()` and `microsecondDelay()` as this upsets the timing needed to 
pay attention to the I2C bus.  I use these tools to control the signal toggling needed to mimic
the IR codes that control the RS.  Fortunately there are a couple of 
[potential workarounds](https://github.com/rambo/TinyWire/issues/8) and the 
empirical fact that the timing needed to control the RS is not terribly sensitive. Once
this is implemented we'll see what we can get away with.

