from bluepy.btle import UUID,Peripheral

print ("serial send test")

 
temp_uuid = UUID(0x0002)

p = Peripheral("C6:78:40:29:EC:31", "random")



try:
   ch = p.getCharacteristics(uuid=temp_uuid)[0]
#   print (stri)
   
   ch.write('AT+DT=215000\r\n')
#   ch.write('AT+TIMEZONE=213800\r\n')

finally:
   p.disconnect()
