# D1mini-BME280-Weather-Monitor

The BME280 will be attached like this:
3v3-VIN
GND-GND
D1-SCL
D2-SDA

The D1 Mini will be loaded with a web browser based OTA sketch, for remote firmware updates.

It will poll the BME using I2C and the BME280 library from Tyler Glenn.
Once it has received the latest data, it will report to thingspeak.
