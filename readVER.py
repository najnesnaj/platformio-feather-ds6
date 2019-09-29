from bluepy.btle import UUID,Peripheral

print ("serial send test")

 
tempr_uuid = UUID(0x0002)
p = Peripheral("F6:2C:86:BD:4A:05", "random")
#p = Peripheral("C6:78:40:29:EC:31", "random")


try:
   cr = p.getCharacteristics(uuid=tempr_uuid)[0]
#    print binascii.b2a_hex(ch.read())
#   print (stri)
   print ('test lezen') 
   print (cr.read())

finally:
   p.disconnect()
