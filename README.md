# second-door

[![Build Status](https://travis-ci.com/ubilab-escape/second-door.svg?branch=master)](https://travis-ci.com/ubilab-escape/second-door)

The main concept is visualized in the figure below.

<p align="center"> 
<img src="/docu/readme/second_door_main_concept.jpg" width=500>
</p>
<p align="center">Figure 1: Main concept<p align="center">

To open the second door, 3 puzzles must be solved.
These puzzles are called Stages in the following.

# Stage 1 - Breaking the lock of the fusebox

In the first stage the group has to crack the lock of the fusebox. A laser somewhere in the room is to be used. In order to make the laser beam visible, a spray bottle, a fog machine or a kind of gas can be used. There is a lock on the fusebox which can only be opened when the laser beam is aimed at it. To do this, the group must use mirrors to divert the laser beam through the entire room. The 4 mirrors that can be used are mounted on a wall with magnets. This hides them, and the group must come up with the idea of using the mirrors.

<p align="center"> 
<img src="/docu/readme/escape_room.png" width=400>
</p>
<p align="center">Figure 2: Room with laser<p align="center">

The lock on the fusebox measures the duration or intensity of the incident laser beam. So that only red laser light can open the lock, appropriate color filters must be attached. For example, a photodiode can be used to detect the incident light beam. The use of a small microcontroller is recommended for the readout of the sensor. The progress of this crack process is visualized with LEDs arranged in a circle. Once this color circle is closed, the microcontroller gives a signal to an actuator so that the lid of the fusebox opens. This completes the first stage.

<p align="center"> 
<img src="/docu/readme/Laser_sensor.png" width=200>
</p>
<p align="center">Figure 3: Laser lock<p align="center">

Hints: Laser warning symbols on lock and laser

# Stage 2 - Restoring the power supply

For the realization of the fusebox we would like to develop our own PCB. A first draft for this PCB can be seen in the following picture.

<p align="center"> 
<img src="/docu/readme/second_quiz_pcb.jpg" width=800>
</p>
<p align="center">Figure 4: PCB fusebox<p align="center">

In the upper left corner you can see an ESP32, which is responsible for the entire control of the PCB. Next to the ESP32, you can see 4 pin header. Both pairs must be connected using jumper wire. The coding for the first pair is hidden in the color marking of a soldered resistor. The coding for the second pair is in a printed code, which can be interpreted binary. Next to the pinheaders there is a seven-segment display with 4 digits. With the help of potentiometers, a number can be set here (e.g. the year in which the faculty was founded). These are the puzzles that must be solved. The progress is visualized by an LED matrix on the right side of the PCB. In addition to the puzzles, there are other components on the PCB to confuse the group. When all the puzzles are solved, the cat flap will be powered again. You can see this with LED-stripes around the door.

Hints: Printed codes on the PCB, resistors, etc.

# Stage 3 - Breaking door by robot

It is just possible to open the door from inside by pushing a button with a small robot. The controller of the robot is placed on the other side of the room and a small window to look inside the server room is at the door. Therefore one participant has to guide the controlling one (swap bottom and top or swap left and right).

There are different possible stories (not decided so far):

* rewiring automatically opens the cat flap (in memory of the "poolkatze") and participants can drive with the robot though it
* simple cat flap which gets unlocked by rewiring and has to be pushed manually
* robot is already inside the server room - rewiring activates lights inside the server room and now the robot can be controlled
