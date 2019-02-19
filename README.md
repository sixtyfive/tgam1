# Teegawhat?

The TGAM1 is an EEG board manufactured by a company called NeuroSky. As far as I can tell, it's difficult (albeit not impossible, for those investigative enough) to buy the thing directly. But then they've been famously used by Mattel in a ca. 2010 game called MindFlex, where you "control" a little foam ball with your thoughts. The headsets can be converted into simple, but cheap 1-channel EEG devices.

A number of websites and projects exist that have done so or are providing instructions. The ones I'm aware of are:

- https://rootsaid.com/getting-raw-data-mindflex-using-arduino/
- https://rootsaid.com/mindflex-arduino-mind-controlled-robot/
- http://www.frontiernerds.com/brain-hack
  - https://github.com/kitschpatrol/Brain
  - https://github.com/kitschpatrol/BrainGrapher
- http://www.zibri.org/2009/09/success.html
- https://hackaday.com/2009/10/20/brain-control-for-the-arduino/
- https://hackaday.com/2009/11/07/mindflex-teardown/
- https://github.com/gpascualg/TGAM1-EEGReader

# So why?

Because all those projects use Arduinos, some with a cable, some with BlueTooth. I wanted more freedom than that and had an ESP8266 lying around. So to make good use of its WiFi connection, this project uses MQTT to extract the raw EEG data and publish it to a message broker. There's also the option to output the full data (pre-calculated EEG power bands as well as what NeuroSky call "eSense") as CSV, again via MQTT, or via serial.

# And how?

Check out the links above, they have many pretty pictures! You may want to try getting the TGAM1 into 57.6k Baud mode with serial commands first. For me that didn't work (it seems that's easier to do with a firmware 1.7 device, but mine has 1.6) so I ended up setting transmission speed in hardware. There's also some documentation about that in the source. So read the source, Leia.
