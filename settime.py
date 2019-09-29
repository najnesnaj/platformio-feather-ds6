from bluepy.btle import UUID,Peripheral
import datetime
import struct
i = datetime.datetime.now()

print ("Current year = %s" %i.year) 
print ("Current month = %s" %i.month)
print ("Current date (day) =  %s" %i.day)
print ("Current hour = %s" %i.hour)  
print ("Current minute = %s" %i.minute)

 
temp_uuid = UUID(0x1234)
#p = Peripheral("C6:78:40:29:EC:31", "random")
p = Peripheral("F6:2C:86:BD:4A:05", "random")



try:
   ch = p.getCharacteristics(uuid=temp_uuid)[0]
   byear = struct.pack("B", (i.year - 2000)) 
   bmonth = struct.pack("B", i.month) 
   bday = struct.pack("B", i.day) 
   bhour = struct.pack("B", i.hour) 
   bminute = struct.pack("B", i.minute) 
   stri = byear + bmonth + bday + bhour + bminute 
#   print (stri)
   ch.write(stri)   

finally:
   p.disconnect()
