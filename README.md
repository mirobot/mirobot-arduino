Mirobot
=======

This is an Arduino library that allows you to control Mirobot (http://mirobot.io).

It can be used in one of two ways;
 - Controlling Mirobot directly from Arduino code (e.g. mirobot.forward(100)). The logic for how it moves is in the Arduino sketch. See the "basic_example" in the examples directory.
 - Receiving commands remotely. The logic for how it moves is somewhere else, most probably in the browser. See the "socket_example" in the examples directory.
