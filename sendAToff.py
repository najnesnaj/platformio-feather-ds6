from bluepy.btle import UUID,Peripheral

print ("serial send test")

 
temp_uuid = UUID(0x0001)
#p = Peripheral("C6:78:40:29:EC:31", "random")
p = Peripheral("F6:2C:86:BD:4A:05", "random")



try:
   ch = p.getCharacteristics(uuid=temp_uuid)[0]
#   print (stri)
   
   ch.write('AT+MOTOR=1\r\n')

finally:
   p.disconnect()
