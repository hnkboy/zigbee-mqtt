#!/usr/bin/python3

import socket
import time
#Create socket object
clientsocket = socket.socket(socket.AF_INET,socket.SOCK_STREAM)

#host = '192.168.1.104'
host = socket.gethostname()
print (host)
port = 8159

clientsocket.connect(('192.168.18.1', port)) #You can substitue the host with the server IP
message = 'hello! Thank you for connecting to the server' + "\r\n"
time.sleep(1)
#clientsocket.sendall(message.encode())


#两种方式
clientsocket.sendall(b"\x78\x78\x11\x01\x07\x52\x53\x36\x78\x90\x02\x42\x70\x00\x32\x01\x00\x05\x12\x79\x0D\x0A")
i = "\x78\x78\x11\x01\x07\x52\x53\x36\x78\x90\x02\x42\x70\x00\x32\x01\x00\x05\x12\x79\x0D\x0A"
clientsocket.sendall(i.encode())
time.sleep(1)
if ord(i[2]) == 0x11:
    print("ok\n")

s =''
s = s + chr(0x42)
s = s + chr(0x43)
s = s + chr(0x44)
s = s + chr(0x45)
s = s + chr(0x46)
clientsocket.sendall(s.encode())


a = ":".join("{:02x}".format(ord(c)) for c in i)
print(a)
#Receiving a maximum of 1024 bytes
message = clientsocket.recv(1024)

clientsocket.close()
