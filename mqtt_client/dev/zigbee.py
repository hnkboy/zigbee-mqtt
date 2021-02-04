import socket
#from enum import Enum

devs = []

class Dev(object):
    def __init__(self, id, address):
        self.id = id
        self.address = address
        print("calss dev id:",self.id,self.address)

    def get_dev(self):
        print("calss dev id:",self.id,self.address)

class Switch(Dev):
    def __init__(self, id, address):
        super(Switch, self).__init__(id, address)
        self.set = 'OFF'
        self.state = 'OFF'
        print("state:" + self.state + "set:" +self.set)
    def set_on(self):
        print("set on")
    def set_off(self):
        print("set on")
    def get_status(self):
        print(self.state)

#if __name__ == '__main__':
print("hi")
switch1=Switch(1,'192.168.0.1')
switch2=Switch(1,'192.168.0.2')
devs.append(switch1)
devs.append(switch2)
for zigee_dev in devs:
    #zigee_dev.set_on()
    zigee_dev.get_dev()
