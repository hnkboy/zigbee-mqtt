import socket
import threading
import asyncio

from infra.config import global_config
from infra.logger import logger
import infra.tcp as zigee_tcp
import infra.mqtt as zigee_mqtt



if __name__ == '__main__':
        #Create new server socket
    host = global_config.getRaw('config','host')
    port = int(global_config.getRaw('config','port'))
    mqtt_host = global_config.getRaw('config','mqtthost')

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind((host, port))
    sock.listen(5)
    #开始监听TCP,zigbee设备wifi tcp连接用
    newConnectionsThread = threading.Thread(target = zigee_tcp.newconnections, args = (sock,), name="tcpserver")
    newConnectionsThread.start()
    #mqttThread = threading.Thread(target = zigee_mqtt.mqtt_start, args = (mqtt_host,), name="mqtt")
    #mqttThread.start()
    asyncio.get_event_loop().run_until_complete(zigee_mqtt.uptime_coro(mqtt_host))


    #必须放最后
    newConnectionsThread.join()
    #mqttThread.join()



