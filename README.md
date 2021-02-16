# SERPICO

**S**tudying **E**lectronics and **R**obotics with **PICO**, is a project of Robot based on **Raspberry PI Pico** board.
This code use two PWM Slice, both channels for each slice, one of which with inverted logic, are used to control a **DRV8833** that drives the two DC motors.

![SERPICO](media/serpico.jpg)

## Robot Car Chassis Kit

I used to make the Robot, one of those kits that can be found for a few euro on almost all online stores.
The kit contains everything you need, the base, the motors, the wheels and the wheel. I added a top made with PVC to house the battery in the lower floor, but other solutions can be found in this regard.

![Robot Car Chassis Kit](media/base-robot-kit.jpg)

## Power

For the power supply of the Robot I used a power bank with two outputs, one of which is connected to the Raspberry PI PICO, via the micro USB connector.
For the other output, I used an old USB cable that I connected directly to the breadboard, to supply the 5V voltage to the motor driver.
**NOTE:** I suggest to use for the power supply of the motors, a power bank that provides more than 1A at the output.

## Circuit

To simplify as much as possible, the realization of the Robot, the circuit is mounted on a bread-board.

![Circuit on bread-board](media/bread-board.jpg)

In the next table the connection between Pico and DRV8833:

|Pico GPIO|DRV8833|
|---|---|
|GPIO 0|IN2|
|GPIO 1|IN1|
|GPIO 2|IN4|
|GPIO 3|IN3|

Other connection of DRV33:

|DRV8833|Motors|USB Power Bank|
| --- | --- | --- |
|VCC||5V USB|
|GND||GND USB|
|OUT 1|Right Motor||
|OUT 2|Right Motor||
|OUT 3|Left Motor||
|OUT 4|Left Motor||

## Next step

The next step is to use an HC-SR04 ultrasonic sensor to identify any obstacles.
