# attiny-$I^2C$Peripheral

Although this project has a specific purpose it can serve as a model for other $I^2C$ 
implementations as this one will entertain many special details with timing.

The original idea was to make the Robosapien (RS) controller an $I^2C$ device that accepts the 1 byte commnands
to control the robot. This allows 3.3V as well as TTL level controls which allows us to 
replace the the primary processor.  This worked well but was extremely simple. I set the address and simply sent the bytes I wanted to send to the IR controller.  The second iteration was a little richer in terms of what I wanted to control and revealed a lot of what I misunderstood about the 'protocol' of how an $I^2C$ controller-peripheral interact.

## Framework
I won't rehash how $I^2C$ devices work when designed from pure hardware. We are using a programmable device running software that handles much of the timing, reading and writing of synchronous data packets. This is about what $I^2C$ devices look like from the controller's perspecive and dictates how you think about writing communications from any number of languages (currently I have used C++, python and node [typescript and javascript])  I got a lot of help from a practical [website on $I^2C$](https://www.robot-electronics.co.uk/i2c-tutorial)

![Clock Cycles on I2C](include/i2cd.gif)

Left shifting the 7 bit address is done to make room for a flag to indicate whether we are reading or writing. To write to address 0x21, you must actually send out 42. Think of the $I^2C$ bus addresses as 8 bit addresses, with even addresses as write only, and the odd addresses as the read address for the same device. The library we are working with takes care of this.  

>**Decide on a device address**:
$I^2C$ devices are first assigned a device address on the synchronous bus either by jumpers or by software. Each device needs a unique address (some devices have special addresses for resets and other special controls but this is relatively rare.)  The address space in our world is 7bit so setting asside 00 as an address this leaves us with the possability of 127 devices. 

>**Create internal register**:
Almost all the documentation for an $I^2C$ device presumes you are poking binary data into specific internal registers that is then read by the electronics of the device to change the state of the system. If the R/W bit is set to read then the controller is announcing that it wants to read from this internal register.

>**Decide what to do**:
The register has to trigger some action. With my Robosapien controller all it did was toggle the data straight out to the receiver in the Robo's head.  With the laser below I have intensities, modes and frequencies to send and receive.

## Implementation
The ATTiny implementation I will be using is Rambo's TinyWireS library which already comes
with an issue using `delay()` and `microsecondDelay()`$^\dagger$ as this upsets the timing needed to 
pay attention to the $I^2C$ bus.  I use these tools to control the signal toggling needed to mimic
the IR codes that control the RS.  Fortunately there are a couple of 
[potential workarounds](https://github.com/rambo/TinyWire/issues/8) and the 
empirical fact that the timing needed to control the RS is not terribly sensitive. Once
this is implemented we'll see what we can get away with.

This is a good model for generalizing the $I^2C$ controller. It sets up multi-byte commands and returns.  
More research revealed that I was working harder than I had to on the ATTiny,

  1. Set the FUSEs for 8MHz.
  2. Decide on an $I^2C$ address for the "Peripheral"
  3. Decide on Internal Register a.k.a. protocols for communications.

Debugging is tricky but you can return state information in a response.  Visual feedback (bi-stable LED) is helpful.

Latest version works to control brightness, alternate from Steady, Blink, and Pulse control.  

I have implemented varying instruction length with success but what seems to work most reliably is keeping your instruction protocol simple and straightforward.  

## Driving a laser pointer (could be any diode).

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
