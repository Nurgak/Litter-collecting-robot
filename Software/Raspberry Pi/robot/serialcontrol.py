#!/usr/bin/python
import sys
import serial
import pygame
import time

ser = serial.Serial(port="/dev/ttyACM0", baudrate=9600)

ser.write('t' + chr(880 >> 8) + chr(800 & 0xff))
ser.write('l')
time.sleep(2)
ser.write('l')

'''while True:
	value = input()
	if value == 27:
		break
	else:
		print "Character: " + value'''

#ser.write("i")
#ser.write(chr(0))
#print ser.readline()
ser.close()
