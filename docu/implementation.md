# Implementaion

In this file we are going to show and document the main implementation tasks.

# General

<p align="center"> 
<img src="implementation/communication_structure.jpg" width=400>
</p>
<p align="center">Figure 1: Visualization for the communication structure<p align="center">

# Fuse Box

## Rewiring

Possible boxes:

| Link 	| Size 	| Cost 	|
|---------	|---------------------	|:--------------:	|
| [Ebay](https://www.ebay.de/itm/IKEA-LIXHULT-Schrank-in-weiss-aus-Metall-25x25cm-Wandschrank-Haengeschrank-/262619576068) 	| 25x25x25 	| 23€ 	|
| [Amazon](https://www.amazon.de/HomeDesign-Medikamentenschrank-Zylinderschloss-Medizinschrank-Hausapotheke/dp/B00GRVG9GA/ref=sr_1_26?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&crid=2ZUS32CP29Z43&keywords=medikamentenschrank&qid=1575026724&sprefix=medik%2Caps%2C178&sr=8-26) |  36x31,5x11 | 48€ |
|[Cablematic](https://cablematic.com/de/products/medikamentenschrank-stahl-medizinschrank-arzneischrank-metall-wandschrank-322-x-140-x-361-mm-KC012/)| 32x14x36 | 23€ |
|[Hornbach](https://www.hornbach.de/shop/Haengeschrank-Kuepper-Hammerschlag-Silber-400-mm-1-Tuer-2-Boeden/5045295/artikel.html)| 40x60x19| 60€ |
|[Ikea](https://www.ikea.com/de/de/p/lillangen-wandschrank-weiss-40240791/)| 40x60x24| 29€ |
|[Amazon](https://www.amazon.de/dp/B007K51EBM/?tag=easysuche-21)| 30x40x15| 49€|


## Door

The laser is used to melt the lock. [LED ring](https://www.adafruit.com/product/2855) is used to indicate the status. 

<p align="center"> 
<img src="implementation/fusebox/led_ring.jpg" width=400>
</p>
<p align="center">Figure X: LEDs for indicating status of melting lock process<p align="center">

# Laser + Box

[Laser](https://www.adafruit.com/product/1054) 

# Button to open door

To open the door a button has to be pushed with the robot. The button needs to communicate with the rest of the room. To give a hint it should be possible to light it up.

<p align="center"> 
<img src="implementation/button/red_push_button.jpg" width=300>
</p>
<p align="center">Figure X: Push button to open the door inside the server room<p align="center">

# Cat flap

Not decided yet!

**Problem**: sliding door does not allow much space for stepper/linear track/..

Possible solutions:

* FuseBox switches on the light inside the server room -> now robot can be seen inside the room
* usual cat flap with small lock

# Robot with controller

The robot will be controlled with an wireless PS2 controller which is attached to the wall. There exists a [library](http://www.billporter.info/2010/06/05/playstation-2-controller-arduino-library-v1-0/) to read out the commands.

<p align="center">
  <img src="implementation/robot/ps2_controller.jpg" width="200" />
  <img src="implementation/robot/ps2_controller_pinout.png" width="264" /> 
</p>

<p align="center">Figure X: PS2 controller wireless with pinout<p align="center">

As chassis for the robot an old prototype from the university is used(Picture will follow). There is already some electronic to controll the two dc-motors.

<p align="center"> 
<img src="implementation/robot/engine_control_robot.png" width=400>
</p>
<p align="center">Figure X: Pinout for existing robot electronic<p align="center">

The pins "motor A1" and "motor A2" are used to controll the left wheel of the robot. Pins "motor B1" and "motor B2" for the right wheel. A LED is used to visualize if the battery of the robot is low. Furthermore there are LEDs "arround" the robot to attract attention of the participants.