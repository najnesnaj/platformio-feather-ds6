import binascii
from bluepy.btle import UUID, Peripheral
 
temp_uuid = UUID(0x2A19)
  
p = Peripheral("C6:78:40:29:EC:31", "random")
   
try:
   ch = p.getCharacteristics(uuid=temp_uuid)[0]
   print binascii.b2a_hex(ch.read())
#   if (ch.supportsRead()):
#         while 1:
#             print ("test")
#             print (ch.read())
#   else: 
#       print ("no result")                                                                   
finally:
    p.disconnect()
