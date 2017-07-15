The Marceline Robot Arduino Library
===================================
Introduction
------------
This is an Arduino library that allows you to control the Marceline Robot https://www.1millionrobots.com

This library is a fork of the Mirobot library. The Mirobot product is at http://mirobot.io

About
-----
This library can be used in one of two ways;
 - Controlling Marceline directly from Arduino code (e.g. mirobot.forward(100)). The logic for how it moves is in the Arduino sketch. See the "basic_example" in the examples directory.
 - Receiving commands remotely. The logic for how it moves is somewhere else, most probably in the browser. See the "socket_example" in the examples directory.
 
When receiving commands remotely the socket can either be used raw or as a websocket from a browser.

It uses the SHA1 and Base64 libraries from https://github.com/ejeklint/ArduinoWebsocketServer in order to provide the Websocket functionality.

How to download this Arduino library so that it can be used with the Arduino IDE:
---------------------------------------------------------------------------------
(1) In your Arduino sketches folder, create a new folder called "libraries" (if you don't already have this folder).
(Your Arduiono sketches folder is shown in the Arduino IDE in File - Preferences - Sketchbook location.)

(2) Copy this library to your "libraries" folder using one of the following two methods:

(i) Download this repository as a zip file and then unzipping into the libraries folder.

(ii) Alternatively, clone the repository into the libraries folder using Git - for example, if you are using Windows and Git Bash:
In Windows Explorer, right click on the libraries folder and do "Git Bash Here". This opens the Git Bash console window.
Clone the library to your library folder. In the console window do:
git clone https://github.com/onemillionrobots/marceline-arduino.git

(3) Now when you open the Arduino IDE the next time, you will be able to see marceline-arduino as a library (in File - Sketchbook - library).
