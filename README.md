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
- https://www.engineersgarage.com/articles/understanding-neurosky-eeg-chip-detail-part-213
- https://github.com/gpascualg/TGAM1-EEGReader
  - https://www.slideshare.net/ssbhamra/eeg-pres
- https://easychair.org/publications/open/tX4S
- https://www.researchgate.net/publication/301531979_Wireless_EEG_Signal_Acquisition_and_Device_ControlEnter_title
- http://pubs.sciepub.com/jcsa/3/5/1/index.html
- http://rdmodernresearch.org/wp-content/uploads/2015/09/21.pdf
- https://www.pantechsolutions.net/blog/wp-content/uploads/2017/10/Brain-controlled-wheel-chair-using-Labview.pdf

# So why?

Because all those projects use Arduinos, some with a cable, some with BlueTooth. I wanted more freedom than that and had an ESP8266 lying around. So to make good use of its WiFi connection, this project uses MQTT to extract the raw EEG data and publish it to a message broker. There's also the option to output the full data (pre-calculated EEG power bands as well as what NeuroSky call "eSense") as CSV, again via MQTT, or via serial.

# And how?

Check out the links above, they have many pretty pictures! You may want to try getting the TGAM1 into 57.6k Baud mode with serial commands first. For me that didn't work (it seems that's easier to do with a firmware 1.7 device, but mine has 1.6) so I ended up setting transmission speed in hardware. There's also some documentation about that in the source. So read the source, Leia.

As for the firmware, first copy `src/config_example.h` to `config.h` and make changes as you see fit. Then you'll want to install PlatformIO and then run `pio run -t upload`.

Once the device is sending messages, `tty2csv` (requires Ruby and the _serialport_ gem; `cd script/ruby; bundle install`) can be used to write the received data to disk or `mqttplot` (requires Python 3.6 and the _matplotlib_ and _paho-mqtt_ packages; `cd script/python; pipfile install`) can be used to watch the EEG signal scroll by. There is another script using MATLAB, but that could use some love. PRs for all three of these very welcome, but I'll also happily accept examples / visualisation scripts / etc. for other languages.

![Screenshot of matplotlib-based Graph](doc/screenshot.png)
