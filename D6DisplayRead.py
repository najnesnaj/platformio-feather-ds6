#by ATC (atcnetz.de)
# Library pygame is needed, pip install pygame
#
#openocd launch command: openocd -d2 -f interface/stlink-v2.cfg -f target/nrf52.cfg
#
#This short programm is used to display the content of the Cheap D6 Tracker on the PC screen, more infos here:
# https://github.com/fanoush/ds-d6
#

import sys
import telnetlib
import re
import pygame
import time

white = (255,255,255)
black = (0,0,0)

lookupTable = {
				"0": "0000", "1": "0001", "2": "0010", "3": "0011", "4": "0100",
				"5": "0101", "6": "0110", "7": "0111", "8": "1000", "9": "1001",
				"a": "1010", "b": "1011", "c": "1100", "d": "1101", "e": "1110",
				"f": "1111", "A": "1010", "B": "1011", "C": "1100", "D": "1101",
				"E": "1110", "F": "1111"
			};
			
colors = {"0": black, "1": white};

pygame.init()
gameDisplay = pygame.display.set_mode((400,300))
gameDisplay.fill(black)
pixAr = pygame.PixelArray(gameDisplay)

tn = telnetlib.Telnet("127.0.0.1",4444,timeout = 4)

def getHEX():
	tn.write("mdb 0x20006ADC 512\n")
	buffer = ''
	while True:
		currentAnswer = tn.read_until("\n")
		if currentAnswer[0:7] == '0x20006':
			buffer += re.sub(r"[^a-fA-Z0-9]+", "", currentAnswer[12:-1])
		
		if currentAnswer[0:10] == '0x20006cbc':
			break
	return buffer


def getBits(bufferarg):		
	bufferarg1 = ''
	for letter in bufferarg:
		bufferarg1 += lookupTable[letter]
	return bufferarg1
		
def drawImage(bitbuffer):	
	page = 0
	x = 0
	y = 7
	for bit in bitbuffer:
		pixAr[x][(page*8)+y] = colors[bit]
		y = y - 1
		if y < 0:
			y = 7
			x = x + 1
			if x >= 128:
				x = 0
				page = page + 1
				
def main():
	while True:
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				pygame.quit()
				quit()
		drawImage(getBits(getHEX()))
		time.sleep(0.2)
		pygame.display.update()
	tn.close

if __name__ == '__main__':
    main()
