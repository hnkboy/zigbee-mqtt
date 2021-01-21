
import asyncio
from hbmqtt.client import MQTTClient, ClientException
from hbmqtt.mqtt.constants import QOS_1, QOS_2
from infra.logger import logger

def mqtt_core(mqtt_host):
    #logger.error("mqtt test %s", mqtt_host)
    print("test\n")
    C = MQTTClient()
    C.connect(mqtt_host)
    # Subscribe to '$SYS/broker/uptime' with QOS=1
    # Subscribe to '$SYS/broker/load/#' with QOS=2
    C.subscribe([
            ('zigbee1', QOS_1),
            ('zigbee2', QOS_2),
         ])
    while True:
        try:
            for i in range(1, 100):
                message = yield from C.deliver_message()
                packet = message.publish_packet
                print("%d:  %s => %s" % (i, packet.variable_header.topic_name, str(packet.payload.data)))
            C.unsubscribe(['zigbee1', 'zigbee2'])
            C.disconnect()
        except ClientException as ce:
            logger.error("Client exception: %s" % ce)


@asyncio.coroutine
def uptime_coro(mqtt_host):
    C = MQTTClient()
    yield from C.connect(mqtt_host)
    # Subscribe to '$SYS/broker/uptime' with QOS=1
    # Subscribe to '$SYS/broker/load/#' with QOS=2
    yield from C.subscribe([
            ('zigbee1', QOS_1),
            ('zigbee2', QOS_2),
         ])
    try:
        for i in range(1, 100):
            message = yield from C.deliver_message()
            packet = message.publish_packet
            #print("%d:  %s => %s" % (i, packet.variable_header.topic_name, str(packet.payload.data)))
            logger.info("%d:  %s => %s" % (i, packet.variable_header.topic_name, str(packet.payload.data)))
        yield from C.unsubscribe(['zigbee1', 'zigbee2'])
        yield from C.disconnect()
    except ClientException as ce:
        logger.error("Client exception: %s" % ce)
