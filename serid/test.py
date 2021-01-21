
#test.py
from ctypes import *
import json
# adddll = cdll.LoadLibrary('/root/Projects/test/add.so')
# adddll.addchar.argtypes = [c_char_p]
# adddll.addchar.restype = c_char_p
# addres = adddll.addchar(b"NLP")
# print(addres.decode("utf8"))

strl =''
strl = strl + chr(0x42)
strl = strl + chr(0x43)
strl = strl + chr(0x44)
strl = strl + chr(0x45)
strl = strl + chr(0x46)

val = bytes(strl,encoding='utf-8')

transbuf = create_string_buffer(512, "\0")#创建512字节内存块，存放动态库函数接口结果
adddll = cdll.LoadLibrary('./libseri.so')
adddll.seri_to_json.argtypes = [c_char_p, c_int, c_char_p]#so库addf函数参数类型
try:
    adddll.seri_to_json(val, len(strl) ,transbuf)
except Exception as e:
    print(str(e))

#print(transbuf.value.decode("utf8"))
print(transbuf.value.decode("utf8"))
#把json字符串转化为python字典
data2 = json.loads(transbuf.value.decode("utf8"))
print(data2['text'])

#把python字典对象转化为json字符串
json_str = json.dumps(data2)
val = bytes(json_str,encoding='utf-8')
try:
    adddll.seri_to_json(val, len(json_str) ,transbuf)
except Exception as e:
    print(str(e))

