使用帮助

1.先启动mqtt-zigee,注意config.ini定制的端口参数

$ python main.py

2.启动serid进程，连接mqtt-zigee，并启动监听，第一个参数为自己做服务器监听的端口，第二个为mqtt-zigee的端口

./serid 8154 8159

3.测试
用tcp客户端连接8154 发送16进制
fe 01 00 05 de af ee ee dd dd 23 00 00 00 12 34 0c 03 01 02 44
可以看到mqtt-zigee 和 serid都打印出json格式的信息