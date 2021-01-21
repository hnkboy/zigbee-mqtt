from scapy.all import *
from scapy.layers.inet import IP, TCP, UDP, Ether
import random

iface = "VMware Virtual Ethernet Adapter for VMnet10"
# VARIABLES
src = "192.168.18.2"
dst = "192.168.18.1"
sport = int(1500)
dport = int(8159)


def send_packet(protocol=None, src_ip=None, src_port=None, flags=None, dst_ip=None, dst_port=None, iface=None,
                message=None):
    """Modify and send an IP packet."""
    if protocol == 'tcp':
        packet = IP(src=src_ip, dst=dst_ip) / TCP(flags=flags, sport=src_port, dport=dst_port) / message
    elif protocol == 'udp':
        if flags: raise Exception(" Flags are not supported for udp")
        packet = IP(src=src_ip, dst=dst_ip) / UDP(sport=src_port, dport=dst_port) / message
    elif protocol == 'eth':
        packet = Ether(dst='52:54:00:3b:81:1a') / IP(src=src_ip,dst=dst_ip,ttl=(3,3)) / message
    else:
        raise Exception("Unknown protocol %s" % protocol)

    #send(packet, iface=iface)
    sendp(packet, iface=iface)

def main():
    num=1
    while True:

        dst_var = str(random.randint(0,255)) + "." + str(random.randint(0,255)) + "." + str(random.randint(0,255)) + "." + str(random.randint(0,255))
        print(dst_var)

        send_packet("eth", src, sport, 'S', dst_var, dport, iface,
                    "Send by Turkay Biliyor : " + datetime.now().strftime("%m/%y %H:%M:%S"))
        time.sleep(1)


if __name__ == "__main__":
    main()