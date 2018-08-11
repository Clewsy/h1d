This is a straightforward implementation of Dean Camera's LUFA drivers starting with his HID keyboard demo.  The hardware is a Pololu A* (A-star) micro with an ATmega32U4.

When connected via USB, the host will recognise the device as a standard keyboard.  A defined string will be "typed" a defined number of times and then... that's it!
