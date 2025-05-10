# DragonWx

DragonWx is a weather display app that receives live telemetry from nearby Home Weather Station sensors using an SDR receiver connected to a computer.

### What You Will Need

- A nearby weather station sensor (could be your own, or even a neighbor's). They periodically transmit current live weather data such as Temperature, Humidity, Windspeed/Direction, etc.
- Computer running either Windows or Linux.
- A functional install of the rtl_433 tuner/decoder tool (https://github.com/merbanan/rtl_433). This open-source tool is what does all the "heavy lifting" in terms of tuning a compatible SDR to receive the pings, and then decoding them into useful human-readable output. Without this tool, my weather app is basically useless. LOL
- An SDR receiver compatible with rtl_433. I'd recommend one of the RTL-SDR variants like the "nooelec". There are also builds of rtl_433 that include the "SoapySDR" driver which adds support for additional SDR devices (such as the SDRplay receivers).

### Setup Instructions

Before using DragonWx, I recommend verifying that you have a working rtl_433 setup by running it first in a terminal to make sure it sees your SDR of choice and is receiving telemetry. rtl_433 can receive and decode a wide range of wireless devices so you'll probably see telemetry from more than just weather stations! It really is a powerful tool! If you are having trouble receiving the pings from your weather station, you may need to tweak/adjust the gain settings of your SDR, but that's beyond the scope of this document. Please check out the <a href="https://github.com/merbanan/rtl_433">rtl_433 GitHub</a> page for more in-depth instructions/information on setting it up.

<br>

<img src="./readme_assets/Screenshot 2025-05-07 005041.jpg">
<!-- Example output from rtl_433 running successfully in the terminal -->
<br>

The first time you launch DragonWx, you will get a message explaining that no config file was found. This is normal since you haven't set anything up yet! Click anywhere on the screen to dismiss the message and click the "Gear" icon to enter the Setup screen. You will need enter in the Sensor IDs for your sensor(s) as well as the full filepath to your rtl_433 executable.

<b>NOTE:</b> Windows clients are able to paste text into fields from the clipboard by left-clicking on a field to select it, and then right-clicking on it to do the paste. This is especially useful for long text fields like the "Exec Path" etc. I hope to support pasting from a Linux clipboard in a future version.

When you are finished, click the OK button. The program will warn you that rtl_433 must be started/restarted for changes to take effect. Click anywhere to dismiss the message and then click "Start RTL433". That should launch the rtl_433 tool in the background which will start looking for your weather sensor telemetry. Click OK again to exit the Setup screen and you should be good to go!

#### Required Fields

- Sensor ID(s) corresponding to your Outdoor and/or Indoor weather sensor
- Path to your rtl_433 executable
- Potentially additional command-line arguments for rtl_433 if your specific SDR requires it (Example: "-d driver=sdrplay" for SDRplay SDRs)

#### Optional Settings

- Custom gain value for your SDR to use
- Temperature and/or Humidity calibration offsets for each sensor
- Toggle between Metric and Imperial units
- Location for internet-based forecast info (in latitude/longitude decimal degrees)
- Reset your weather statistics (like high/low temperature, rainfall data, etc)
- Start/Restart the rtl_433 executable (required for certain changes to go into effect such as a different gain value)

For convience, you can also configure/edit you

### Special Thanks and Attributions

Special thanks to my bestie Kirstin Stich for her invaluable insight/input on the look and feel of the app, and to everyone on the <a href="https://discord.gg/WhwHUMV">OneLoneCoder Discord</a> for all your help, advice, and patience with my coding questions! Thank you very much!