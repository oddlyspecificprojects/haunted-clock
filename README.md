# Haunted Clock!

This project is a Halloween idea, inspired by an idea at Hackaday of a drunken clock.

Take a look at the [schematic](doc/v0.1.pdf) for the impression.


## Hardware

* ATMEGA168 (I use an arduino and just flash without all of the libraries)
* Clock module based on TM1637 (6 digit 7 segment display controller)
* 6*7 segment digits _common anode_ - can be a 4 digit unit with the colons between digits.
* If no colon between digits available, 4 RED leds
* DS1307Z RTC module (that has 64k eeprom on it)
* A green led
* Small buzzer
* 3 momentary buttons
* 3 SPDT switches (I use small sliding ones)
* Wires, protoboard and other tools


## (Planned) Features

A clock, after all, haunted or not this is still a clock.  
Also, it has an alarm built-in, so it beeps when set to.
To control everything, there are 3 interface buttons.

To disable features (glitches, buzzer, and turn off the leds), there are switches at the back.

But, because it is haunted, this can happen:

*	"Sticky" digit - a led that has turned on stays on
*	Counts past 60 - quite obvious
*	Random flickers
*	Stops counting and then counts really fast to catch up
*	Starts counting backwards
*	Colons LED blink out of sync
*	Digit malfunctions
*	Beeps with disregard to the alarm


## Implementation order

* Make the leds work for real
* Add the RTC so it can display the time
* Add Alarm (Should be implemented inside the RTC)
* Add interface buttons
* Add control switches
* Fun part - glitches!!!

Use git actions for compilation maybe?

