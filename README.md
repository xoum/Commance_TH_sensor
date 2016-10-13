# Commance TH_sensor
TH_sensor is a battery powered Temperature and Hygrometric sensor sending data wirelessly through 433MHz for domotic application.

You'll find many other implementation of that kind of sensors around the internet. Some of them are based on ATTiny85 and/or Dallas temperature sensors. However, I hope that my implementation will be usefull to the community

## Hardware
This project is based on:
- An Arduino Pro Mini board or Arduino Nano board,
- A FS1000A 433MHz radio module,
- A GY-21 sensor based on [SHT21](https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/Humidity_Sensors/Sensirion_Humidity_Sensors_SHT21_Datasheet_V4.pdf) Humidity and Temperature Sensor IC,
- A 3xAAA battery holder.

In this implementation, the data sent by the TH_sensor are received by a RFXCom that I use for domotic stuff. Since I use [Oregon Scientific RF protocol](http://wmrx00.sourceforge.net/Arduino/OregonScientific-RF-Protocols.pdf) the TH_sensor should be compatible with all the receiver understanding the Oregon protocol.



## Trademark Disclaimer

Product names, logos, brands and other trademarks referred to within the Commance TH_sensor website are the property of their respective trademark holders. These trademark holders are not affiliated with Commance TH_sensor. They do not sponsor or endorse our materials.
