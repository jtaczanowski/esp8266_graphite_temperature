# esp8266_graphite_temperature
[esp8266] Log temperature readings from DS18B20 and wifi signal strength to graphite server.
To log something on graphite server the esp8266 have to send UDP packet with string containg: metric name, metric value, timestamp. Bellow expample of such strings:
esp.temp 21.50 1482075500
esp.rssi.JAN -73 1482075500

![alt tag](https://raw.githubusercontent.com/jtaczanowski/esp8266_graphite_temperature/master/serial_port.png)
![alt tag](https://raw.githubusercontent.com/jtaczanowski/esp8266_graphite_temperature/master/esp8266_graphite_grafana.png)

