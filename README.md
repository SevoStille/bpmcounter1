# bpmcounter1
BPM Counter by arduino timer
/*
 * BPM counter
 * 
 * Small utility to display BPM, clock Hz and ms on a 16x2 I2C LED display 
 * Written for DIN Sync devices like the TR-606.  
 * Counts time between pulses on external timer 
 * 
 * Connect the clock signal to pin 8 (and ground to GND) - e.g. 
 * DIN Sync Pin 3 to Arduino Uno Pin 8, DIN Pin 2 to any GND terminal on the Uno 
 * 
 * recommended External circuit: 
 * 7414 Schmitt Trigger for signal stabilisation, 2.2k pulldown, optionally 
 * optocoupler 
 * 
 * Copyright 2018 Sevo Stille <mailto:sevo@radiox.de>
 * @license GPL-3.0+ <http://www.gnu.org/licenses/gpl.html>
 * 
*/
