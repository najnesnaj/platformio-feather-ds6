from bluepy.btle import UUID,Peripheral

print ("serial send test")

 
temp_uuid = UUID(0x0001)
tempr_uuid = UUID(0x0002)
p = Peripheral("F6:2C:86:BD:4A:05", "random")



try:
   ch = p.getCharacteristics(uuid=temp_uuid)[0]
   cr = p.getCharacteristics(uuid=tempr_uuid)[0]
#    print binascii.b2a_hex(ch.read())
#   print (stri)
   
   ch.write('AT+HRMONITOR\r\n')
   print (cr.read())

finally:
   p.disconnect()
