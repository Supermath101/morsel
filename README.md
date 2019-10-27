morsel
=======

## Overview

This arduino sketch reads button presses from a single button and types the
received letters via BLE HID commands. It is intended to be used on an
Arduino with attached nRF51 such as an Adafruit Feather Bluefruit LE.

## Compile / Upload the sketch

Before building this code, install the "Adafruit AVR Boards" in Boards Manager,
and the "Adadafruit BluefruitLE nRF51" library in Library Manager.

Set constant SWITCH_PIN as desired. The switch should be connected between
SWITCH_PIN and ground. Once this is set, upload the sketch.

On the first run, the BLE module will be reset. The device will show up as a
BLE keyboard called "morsel".

Should you need to start over and delete BLE pairing data, hold the switch
for 10s.

## Calibration

> The calibration algorithm leads to the dit time, dah time, time between
> letters, and time for a space character, to all be adjusted based on how
> fast the user is keying. This hopefully spaces out the interpretation of
> letters if they are keying more slowly.

Start by keying a dit-dah. The calibration algorithm uses the shortest and
longest "on" time (from the last 10 presses) to decide a dit vs dah, based on
a value halfway between the shortest and longest press.

The time the switch is off before interpreting the letter is determined by the
constant BLIP_SLICE, where a value of 1 is least aggressive (equal to the
longest press plus the difference between shortest and longest). Setting this
value to 2 halves the delta, leading to the letter verdict being made sooner.

After interpreting a letter, the time the switch is off before inserting a space
character is determined by BLIP_SPACE. This constant is a muliple of the letter
pause duration, so a value of 2 will send a space after a period equal to twice
the time it takes to decide what letter was typed. Setting this to 3 or 4 will
result in a longer pause before inserting a space.

## CAD models

[Onshape artifacts](https://cad.onshape.com/documents/a5cf00b2b2b8b7f75a416cac/w/fb23e6355082f36a0f72b345/e/1260f466cc52925c32e8d48e)
