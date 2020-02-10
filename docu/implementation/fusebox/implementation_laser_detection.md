Implementation Laser Detection

The laser beam is diverted by the mirrors and hits the detector visible below. The detector is equipped with a photodiode 
and an integrated comparator, which outputs a logical one when it detects a laser beam. 
The high response time of the diode allows to modulate a frequency to the laser and to detect this. The frequency is needed 
to prevent the puzzle from being bypassed by a flashlight. The signal is passed to an input of the ESP32 and this is sampled. 
A sequence of six samples is stored and checked for the equivalent number and order of zeros and ones. 
If this is given, the first LED of the ring is set to green. This is repeated until all sixteen LED's are green. 
Then the LED ring flashes three times to signal that the puzzle is solved. To increase the difficulty, a green LED is set to 
red again after one second to increase the difficulty.
