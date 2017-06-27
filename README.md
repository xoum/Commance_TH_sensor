![Commance](/images/logo_commance.png)](http://www.commance.com) TH_sensor

TH_sensor is a battery powered Temperature and Hygrometric sensor sending data wirelessly through 433MHz for domotic application.

You'll find many other implementation of that kind of sensors around the internet. Some of them are based on ATTiny85 and/or Dallas temperature sensors. However, I hope that my implementation will be useful to the community. I've released a first version of this sensor around a year ago and I'm now decided to force me to publish something on the internet. I would like to apologize to some authors if I used their contributions without quoting them. I have not written all sources of information that I used when I worked on this project. If you think it's relevant to quote your work, let me know!

## Hardware
This project is based on:
- An [Arduino Pro Mini](https://www.arduino.cc/en/Main/ArduinoBoardProMini) board or Arduino Nano board 5V 16MHz,
- A FS1000A 433MHz radio module,
- A GY-21 sensor based on [SHT21](https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/Humidity_Sensors/Sensirion_Humidity_Sensors_SHT21_Datasheet_V4.pdf) Humidity and Temperature Sensor IC,
- A 3xAA battery holder.

In this implementation, the data sent by the TH_sensor are received by a [RFXCom](http://www.rfxcom.com/epages/78165469.sf/en_GB/?ObjectPath=/Shops/78165469/Products/14103) that I use for domotic stuff. Since I use [Oregon Scientific RF protocol](http://wmrx00.sourceforge.net/Arduino/OregonScientific-RF-Protocols.pdf) the TH_sensor should be compatible with all the receiver understanding the Oregon protocol.

![Hardware involved](/images/photo_hardware1.jpg)

*You'll notice the white wire soldered in the corner of the FS1000A radio module. You have to add this if your module does not have any antenna. Its optimal lenght should be quarter the wavelength @433MHz: 17.3cm*
### Autonomy / Low power optimization
The TH_sensor is battery powered and we don't want to change or recharge the cells every days nor every weeks (in fact at the end the 3 AA battery are suitable for 1 year of service). We'll see here after how to reduce the power consumption by using software tricks. But now, what can we achieve on the hardware side?

The biggest current drain is caused by the power LED! So, the first thing to do is to cut the conductive track of the power LED.

Then, the next current drain is caused by the 5V voltage regulator. Provided we are using a 3 cells battery holder we will supply the Arduino through its 5V **VCC** pin directly (not with the RAW one). Doing this, the voltage regulator will not be used **but** it still drains current... The solution is to cut the conductive track linking the output of the 5v regulator to the 5V pin (close to the regulator).  
![details of tracks to cut](/images/zoom_cut.jpg)

### Connections
The pinout of the Arduino Pro Mini can change depending on version. The GY-21 sensor uses I2C for the communication with the Arduino and this data bus involves 2 lines: **SCL** (clock) and **SDA** (data). On the Arduino Pro Mini, the analog pin **A4** is **SDA** and the analog pin **A5** is **SCL**.  
![wiring](/images/wiring.png)

## Software
### Library
- Adafruit_HTU21DF
- [LowPower][LowPower lib]

### Battery tricks

I'm using the LowPower library to put the ATmega328 in sleep mode. You can put the CPU in sleep mode for a maximum of 8 seconds (or forever). After height seconds, the CPU standup and continue to execute the code. When this happen I turn again the CPU in sleep mode for 8 more seconds and I do this between 25 to 31 time. I obtain temperature and hygrometry every 3 to 4 minutes.

Just before putting the CPU in sleep mode, I turn off the VIN of the GY-21 sensor and the VCC of the FS1000A radio module.

The sensor can send a message when the battery is low, I send this message when the 3AA battery pack reaches a total voltage of 3.5 V (under this voltage the 5V 16MHz board doesn't work properly)

To prevent deep discharge of the NiMH elements, I put the board in sleep mode forever when the voltage level goes behind 3.3V

By doing this and hardware modifications, the current drained stays bellow 5µA the most part of the time.
With 3 Alkaline AAA batteries:

| Hardware  | Current  |
|---|---|
| Genuine  | 2.97mA  |
| No LED  | 0.084mA  |
| No LED, NO regul  | 0.005mA  |

### Radio tricks

The only trick I use is to put a jitter on the number of 8 seconds sleeps of the CPU. When I use a high number of sensors, this avoids to have RF (Radio-Frequency) collisions during a long period of time. 
Think that all the cards have not exactly the same clock and if 2 sensors starts to broadcast theirs signal at the same time, with no jitter, they will continue to scramble each other for a long time.


## Improvements
It is maybe a good idea to use a 3.3V 8MHz Pro Mini board instead of the 5V 16MHz one. I've noticed some instability on some cards and I suppose this is due to the fact that the ATmega is not specified to work far under 5V @ 16MHz.

If you need to boost the range of the RF signal, you can add a DC-DC voltage booster between the digital pin 2 and the VCC on the radio module. Those FS1000A modules can go up to 12V.


## Trademark Disclaimer

Product names, logos, brands and other trademarks referred to within the TH_sensor website are the property of their respective trademark holders. These trademark holders are not affiliated with TH_sensor. They do not sponsor or endorse our materials.

## Sources
http://www.connectingstuff.net/blog/encodage-protocoles-oregon-scientific-sur-arduino/  
http://www.instructables.com/id/Secret-Arduino-Voltmeter/  
[LowPower lib]: https://github.com/rocketscream/Low-Power  
