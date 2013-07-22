FailClock
=========

An Arduino project which assembles a day counter which resets as soon as Continuous Integration fails. "XXXX Days without injury"
See the sources for more information about what each file does.

You can find more information about this project in my blog:
http://yetiblog1337.wordpress.com/2013/07/20/failclock-motivation-to-keep-the-code-clean/

You probably need to alter the sources to fit your needs (Python path, COM-interface, ...)

Software Requirements:
- Arduino UNO drivers
- Python 2.x
- PySerial

Hardware Requirements:
- Arduino (I used an UNO, but others will probably work too)
- Adafruit 7 Segment Display + Backpack
- Serial USB connection to a PC which controls everything

