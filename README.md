# stm32f103c8t6_button
An eclipse project for a STM32f103c8t6 arm system.
This project builds on the blink, uart1 and interrupt1 to add 3 buttons
it reads the button state (PA0 PA1 and PC15) and toggles an LED (PB7, PB8 and 
PB9)


it aso sets up two timers to blink pins PB5 and PB6
it is built on the blink and uart1 programs so it will also
initialise the uart1 pins (PB9 and PB10) on an stm32f103c8t6 development board 
and send a message in a continuous loop.
you can use this to help debug your programs 
It also blinks pc13 so you know all is working
this project is built on the PC13 blink program.
 It is the default program when creating a basic "hello world" for 
 the stm32f103.

You can download the whole project and import it into eclipse.
