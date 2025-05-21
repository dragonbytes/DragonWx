# DragonWx

DragonWx is a weather display app that receives live telemetry from nearby Home Weather Station sensors using an SDR receiver connected to a computer.

### What You Will Need

- Computer running either Windows or Linux.
- A functional install of the rtl_433 tuner/decoder tool (https://github.com/merbanan/rtl_433). This open-source tool is what does all the "heavy lifting" in terms of tuning a compatible SDR in to receive the pings from weather sensors, and then decodes them into useful human-readable output. Without this tool, my weather app is basically useless. LOL
- An SDR receiver compatible with rtl_433. I'd recommend one of the RTL-SDR variants like the "nooelec". There are also builds of rtl_433 that include the "SoapySDR" driver which adds support for additional SDR devices (such as the SDRplay receivers).
- A supported weather sensor, which is what performs the measurements and beacons out the telemetry wirelessly to a display (and your SDR). I know for sure that Acurite and Ambient weather sensors work, but again, visit the rtl_433 project page for more in depth information on supported sensors.
- (Optional) A second sensor for <b>Indoor</b> temperature and humidity information.<br><br>
<i>Unlike the original/factory weather station displays, our computers/screens do not have a built sensor to measure the indoor temperature and humidity, so I decided the best way to replicate this functionality was to use a second external sensor. The smaller temperature/humidity-only sensors are usually be pretty inexpensive.</i>

## Setup Instructions

### Confirm Your SDR / rtl_433 Setup Works

Before using DragonWx, I recommend verifying that you have a working rtl_433 setup by running it first in a terminal to make sure it sees your SDR of choice and is receiving telemetry. rtl_433 can receive and decode a wide range of wireless devices so you'll probably see telemetry from more than just weather stations! It really is a powerful tool! If you are having trouble receiving the pings from your weather station, you may need to tweak/adjust the gain settings of your SDR, but that's beyond the scope of this document. Please check out the <a href="https://github.com/merbanan/rtl_433">rtl_433 GitHub</a> page for more in-depth instructions/information on setting it up. Once you are receiving good telemetry, note the ID number of your specific Outdoor sensor (and Indoor sensor if you have one). These IDs will be required during the DragonWx setup process described below.

A simple example, for reference, of using an SDRplay as the device (via SoapySDR) and with a gain setting of 5 dB:
<pre>rtl_433-rtlsdr-soapysdr.exe -d driver=sdrplay -g5</pre>

<br>
<p align="center">
	<img src="./readme_assets/Screenshot 2025-05-07 005041.jpg">
<i>Example output from rtl_433 running successfully in the terminal</i>
</p>
<br>

### Configure DragonWx

The first time you launch DragonWx, you will get a welcome message that directs you on how to enter Setup Mode. Click anywhere on the screen to dismiss the message and then click the "Gear" icon. Here you will need enter in the Sensor IDs for your sensor(s) you are using as well as the full filepath to your rtl_433 executable.

<b>NOTE:</b> Windows clients are able to paste text into fields from the clipboard by left-clicking on a field to select it, and then right-clicking on it to do the paste. This is especially useful for long text fields like the "Exec Path" etc. I hope to support pasting from a Linux clipboard in a future version.

#### Required Fields

- Sensor ID(s) corresponding to your Outdoor and/or Indoor weather sensor(s).<br>(These IDs can be found while running rtl_433 directly in a terminal as described above.)
- Path to your rtl_433 executable
- Potentially additional command-line arguments for rtl_433 if your specific SDR requires it (Example: "-d driver=sdrplay" for SDRplay SDRs)

#### Optional Settings

- Custom gain value for your SDR to use
- Temperature and/or Humidity calibration offsets for each sensor
- Toggle between Metric and Imperial units
- Location for internet-based forecast info (in latitude/longitude decimal degrees)

When you are finished, click the OK button. The program will warn you that rtl_433 must be started for changes to take effect. Click anywhere to dismiss the message and then click "Start RTL433". That should launch the rtl_433 tool in the background which will start looking for your weather sensor telemetry. Click OK again to exit the Setup screen and you should be good to go!

For convenience, you may also configure DragonWx manually by editing `DragonWx.conf`, which is just a plain-text file. The parameters should be pretty self-explanatory.

### Using DragonWx

Once you have successfully configured DragonWx and it is receiving telemetry from rtl_433, the info on the screen should update every time it receives a new ping from your sensor(s). The time between those pings varies between companies and even specific models, but it's usually between 15 to 45 seconds. There are a few clickable areas on the screen that can give you additional information:

- Click the "Feels Like" label to have it instead show which method is being used to calculate the feels-like temperature. (Wind Chill, Heat Index, Steadman Apparent Temperature, or just the ACTUAL current outdoor temperature). Clicking on it again will revert back to the "Feels Like" catch-all label.
- If your weather station has a Light Intensity sensor, you can click on the verbal description of how bright it is outside (Overcast, Direct Sun, Twilight, etc) to display the actual value measure in Lux. Clicking again will revert back to the verbal description.

### Special Thanks

Special thanks and shoutout to my good friend Kirstin Stich for her invaluable insight/input on the look and feel of the app, and to everyone on the <a href="https://discord.gg/WhwHUMV">OneLoneCoder Discord</a> for all your help, advice, and patience with my coding questions! Thank you very much!

### Icon Attributions

<a href="https://bas.dev/work/meteocons">Meteocons by Bas Milius</a>
<br>
<a href="https://www.flaticon.com/free-icons/increase" title="increase icons">Increase icons created by Nur syifa fauziah - Flaticon</a>
<br>
<a href="https://www.flaticon.com/free-icons/arrow" title="arrow icons">Arrow icons created by Dave Gandy - Flaticon</a> (Modified Color / License <a href="http://creativecommons.org/licenses/by/3.0/">CC 3.0 BY</a>)
<br>
<a href="https://www.flaticon.com/free-icons/gear" title="gear icons">Gear icons created by Freepik - Flaticon</a>
<br>
<a href="https://www.flaticon.com/free-icons/close" title="close icons">Close icons created by VectorPortal - Flaticon</a>
<br>
<a href="https://www.flaticon.com/free-icons/signal" title="signal icons">Signal icons created by Ayub Irawan - Flaticon</a>
