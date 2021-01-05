# somfy-esp8266

With this library you can control your Somfy blinds with ESP8266 in three easy steps:

  - Instantiate the class by passing RF pin and the first remote id
    - Somfy somfy(RF_SENDER, 0x134452);
  - Add remotes, with the name as the only parameter
    - somfy.AddRemote("corridor");
    - Now you can send MQTT messages in the format of [name of blind]+somfyCommand[U|D|S|P], e.g. “corridorU”
  - Watch for MQTT messages
    - somfy.ProcessMessageAndExecuteCommand(messageReceivedInYourTopic)


Supports multiple remotes
Persistent rolling code (remotes will be remembered after restarts and uploads)
Very easy to use with MQTT

## Prerequisites
### Hardware
Appropriate RF sender

Somfy RTS remotes operate on 433.42 Mhz. The transmitter commonly sold have a 433.92 MHz crystal. You can buy 433.42 Mhz crystal and change it with a bit of soldering.
### Software

For the example to work, the [Adafruit MQTT library](https://github.com/adafruit/Adafruit_MQTT_Library) need to be included. But any other can be used.

## MQTT messages

Set up your favourite MQTT client (I use MQTT Dash for Android) to send the following messages:

You need four messages per a blind, namely

#### Up
Message:  [name of blind]+U, e.g. “corridorU”
#### Down
Message:  [name of blind]+D, e.g. “corridorD”
#### Stop
Message:  [name of blind]+S, e.g. “corridorS”
#### Prog
Message:  [name of blind]+P, e.g. “corridorP”

## Programming the virtual remotes
Once you have your ESP8266 up and running, here is how you can make use of it:

Long press  the “prog”  button on your original Somfy remote, until the blind that is attached moves up and down a little. That is telling you, that it waits for a remote to connect. Now send the PROG command for your virtual remote (e.g. corridorP). Now the blind should make the same up and down movement again, signaling that the remote is attached. You can still use your original remote, but now you can send the messages Up, Down and Stop from your virtual remote as well.

You can repeat this process for your other blinds. 

Congratulations, you just made your ESP8266 to be capable of moving your blinds!
## Disclaimer

I have a few ESP8266s around the house, and I wanted to control my Somfy blinds through them. A few source codes can be found on the internet (I mention them below), every one of them originating from the same source from Nickduino ([github link](https://github.com/Nickduino/Somfy_Remote)).

I decided to make a library, using the cited sources, and the goal was to make the usage as simple as possible.

I have been using this for a while now (since 2020 november), and works great.
Hope this helps some people. If you have any questions or remarks, feel free to reach out.

### Sources

[https://4x5mg.net/2018/06/10/controlling-somfy-blinds-with-esp8266/](https://4x5mg.net/2018/06/10/controlling-somfy-blinds-with-esp8266/)

[https://github.com/Nickduino/Somfy_Remote/blob/master/Somfy_Remote.ino](https://github.com/Nickduino/Somfy_Remote/blob/master/Somfy_Remote.ino)

[https://nodemcu.readthedocs.io/en/release/modules/somfy/](https://nodemcu.readthedocs.io/en/release/modules/somfy/)

[https://github.com/RoyOltmans/somfy_esp8266_remote_arduino/blob/master/somfy_remote_arduino.ino](https://github.com/RoyOltmans/somfy_esp8266_remote_arduino/blob/master/somfy_remote_arduino.ino)
