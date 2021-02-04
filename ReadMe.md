使用帮助

1.先启动mqtt-client,注意config.ini定制的端口参数

$ python mqtt_clietn/main.py

2.启动serid进程，连接mqtt-zigee，并启动监听，前两个个参数为自己做服务器监听的地址和端口，第三四个为mqtt-client的地址和端口

serid/serid 0.0.0.0 8154 127.0.0.1 8159
serid/serid 0.0.0.0 8154 192.168.32.29  8159
3.测试
用tcp客户端连接8154 发送16进制
fe 01 00 05 de af ee ee dd dd 23 00 00 00 12 34 0c 03 01 02 44
可以看到mqtt-zigee 和 serid都打印出json格式的信息






#测试mqttos
mosquitto_pub  -h "test.mosquitto.org" -t "zigbee1" -m "welcome to MQTT"