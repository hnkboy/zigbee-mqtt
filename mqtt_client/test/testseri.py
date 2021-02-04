from scapy.layers.l2 import Ether
from scapy.packet import Raw
from scapy.layers.inet import IP, UDP,TCP
from scapy.sendrecv import send

#导入scapy的所有功能
from scapy.all import *
print("hello")


#首先要选择网卡的接口，就需要查看网卡接口有什么,在进行选择
print(show_interfaces())

data='hello scapy'
pkt = Ether()/IP(dst='192.168.18.1')/TCP(dport=8159)/data
#可以先通过a.show()函数查看数据包的构成部分，然后在构造包时就知道有哪些参数可以填了
print(pkt.show())
sendp(pkt, inter=1, count=5, iface='VMware Virtual Ethernet Adapter for VMnet10')

#查看抓取到的数据包
pkts=sniff(iface='Realtek PCIe GbE Family Controller',count=3,filter='arp')
print(pkts)

#sans = sr1(IP(dst='192.168.18.1')/TCP(dport=8159,sport=RandShort(),seq=RandInt(),flags='S'),verbose=False)



