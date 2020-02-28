[![Build Status](https://travis-ci.com/ubilab-escape/second-door.svg?branch=master)](https://travis-ci.com/ubilab-escape/second-door)

- [Riddles](#Riddles)
  * [Stage 1 - Breaking the lock of the fusebox](#stage-1---breaking-the-lock-of-the-fusebox)
  * [Stage 2 - Restoring the power supply for the robot](#stage-2---restoring-the-power-supply-for-the-robot)
  * [Stage 3 - Breaking door by robot](#stage-3---breaking-door-by-robot)
- [Communication](#Communication)
- [How to](#How-to)
- [Future work](#future-work)

# Riddles

The main concept is visualized in the figure below.

<p align="center"> 
<img src="/docu/readme/escape_room.png" width=400>
</p>
<p align="center">Figure 1: Main concept<p align="center">

To open the second door, 3 puzzles must be solved. These puzzles are called Stages in the following. In case the participants get stuck they can ask for hints. The implemented hints can be seen [here](/docu/hints.md)

## Stage 1 - Breaking the lock of the fusebox

In the first stage the group has to crack the lock of the fusebox. A laser somewhere in the room is to be used. There is a lock on the fusebox which can only be opened when the laser beam is aimed at it. To do this, the group must use mirrors to divert the laser beam through the entire room. The 2 mirrors that can be used are mounted on a wall. This hides them, and the group must come up with the idea of using the mirrors.

The lock on the fusebox measures the laser beam. So that only this specific laser light can open the lock. The readout of the signal is done by the ESP32 inside the fusebox. The progress of this crack process is visualized with LEDs arranged in a circle. Once this color circle is closed, the microcontroller gives a signal to an actuator so that the lid of the fusebox opens. This completes the first stage.

<p align="center"> 
<img src="/docu/readme/Laser_sensor.png" width=200>
</p>
<p align="center">Figure 2: Laser lock<p align="center">

To make it easier for the participants the laser warning symbol will be attached to the fusebox.

## Stage 2 - Restoring the power supply for the robot

Inside the fusebox two small "rewiring" riddles need to be solved . The following image shows the inside of the fusebox.

<p align="center"> 
<img src="/docu/readme/finished_box.jpg" width=800>
</p>
<p align="center">Figure 3: PCB fusebox<p align="center">

In the middle you can see an ESP32, which is responsible for the entire control of the PCB. Left to the ESP32, you can see two 5 pin headers (marked with 1). Both pairs must be connected using the attached jumper wire. The coding is hidden in the color marking of a soldered resistor which is right next to the connector. Above the ESP32 is a seven-segment display with 4 digits (marked with 2). With the help of potentiometers, a number can be set here (the year in which the faculty was founded). To the right of the ESP32 is the third riddle. Again the jumper wire needs to be connected in the right order. This time a binary number needs to be coded. Black wire indicates a zero and white wire a one. To get the right solution the participants have to sum up two hexadecimal numbers. The progress is visualized by an LED matrix on top of the PCB. In addition to the puzzles, there are other components on the PCB to confuse the group. When all the puzzles are solved, the robot inside the server room will get controllable from outside.

## Stage 3 - Breaking door by robot

It is just possible to open the door from inside by pushing a button with a small robot. Luckily the participants were able to repower the robot in the riddle before. The controller of the robot is placed on the other side of the room and a small window to look inside the server room is at the door. Therefore one participant has to guide the controlling one.

<p align="center"> 
<img src="/docu/readme/robot.jpeg" width=400>
</p>
<p align="center">Figure 4: Robot<p align="center">

# Communication

To standardize the communication for all devices and make the handling as easy as possible we implemented the [MqttBase](https://github.com/JayWiz/MqttBase). Each device for the above riddles make use of this class. The class is embedded as sub repository.

# How to

The project is structured in four folder:

| Folder 	| Description 	|
|------	|---------------------|
[3D-files](https://github.com/ubilab-escape/second-door/tree/master/3D-files) | Contrains all .stp and .stl files for the designed cases |
[code](https://github.com/ubilab-escape/second-door/tree/master/code)| The .io files with all the code for each riddle |
[docu](https://github.com/ubilab-escape/second-door/tree/master/docu) | More detailed information about the implementation |
[pcb](https://github.com/ubilab-escape/second-door/tree/master/pcb) | The schematic and layout for the designed pcb |

The easiest way to work/compile/upload the code is to get [vs code](https://code.visualstudio.com/) and install [PlatfomIO](https://platformio.org/install/ide?install=vscode) as extension. The dependencies of each project are declared within the "lib_deps". The only missing part is the "wifi_secure.h" file which has to be created by the user. The file must contain:

```cpp
const char* ssid = "";
const char* password = "";
```

If you want to get the serial output for connection information define in the main.cpp file:

```cpp
#define DEBUG
```



# Future work

* robot controlled by ESP32 only
* cover/coating for pcb inside fusebox
* coating for fusebox
