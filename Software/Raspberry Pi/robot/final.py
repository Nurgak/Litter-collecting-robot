#!/usr/bin/env python
import time
import sys
from cv2 import waitKey

import comm
import camera

# types of elements to detect

class RobotControl:
	def __init__(self, port, baudrate):
		# configuration for the serial connection to the robot
		self.port = port
		self.baudrate = baudrate
		
	def connect(self):
        # start serial communication thread
		try:
			print "Connecting to robot"
			self.commThread = comm.CommThread(self.port, self.baudrate)
			self.commThread.start()
		except:
			print "Could not connect to robot"
			return
		
		preview = False
		# run the code with or without the preview, by default it's disabled
		if len(sys.argv) > 1:
			if sys.argv[1]:
				preview = True
		
		# start the camera thread
		try:
			print "Starting camera"
			self.cameraThread = camera.ImageProcessing(256, 128, preview)
			self.cameraThread.start()
		except:
			print "Could not start camera"
			return

		# start the control loop
		self.control()
		
	def control(self):
		# wait for the camera and communication threads to start
		while not self.commThread.isAlive() and not self.cameraThread.isAlive():
			time.sleep(0.1)
			pass
		
		print "Starting robot control loop"
		
		# play 880hz sound to indicate scropt start when working without a screen
		print "Beep"
		self.commThread.write('t' + chr(880 >> 8) + chr(880 & 0xff))
		
		# turn on the led lights on the robot
		print "Turning light on"
		self.commThread.write('l' + chr(1))

		print "Enable bottle detection"
		self.commThread.write('0')
		
		while True:
			try:
				self.stateSearching()

				# check for keyboard input
				if self.checkInput():
					break
		
				time.sleep(0.1)
			except (KeyboardInterrupt, SystemExit):
				print "User forced exit"
				break
			except Exception as e:
				# when an error occurs make sure to end the dependant threads
				print e
				self.stopThreads()
				break
		
		# turn off the led lights on the robot
		print "Turning light off"
		self.commThread.write('l' + chr(0))

		print "Disable bottle detection"
		self.commThread.write('0')
		
		print "Control loop exited"
		self.stopThreads()

	def stopThreads(self):
		print "Stopping threads"
		self.commThread.stop()
		self.commThread.join()
		self.cameraThread.stop()
		self.cameraThread.join()

	def checkInput(self):
		# check for user input
		key = waitKey(10) & 0xff
		
		if key == 27:
			print "Manually stopped"
			return True
		elif key == ord('w'):
			self.commThread.write('s' + chr(self.maxSpeed) + chr(self.maxSpeed))
		elif key == ord('a'):
			self.commThread.write('s' + chr(-self.maxSpeed + 255) + chr(self.maxSpeed))
		elif key == ord('d'):
			self.commThread.write('s' + chr(self.maxSpeed) + chr(-self.maxSpeed + 255))
		elif key == ord('s'):
			self.commThread.write('s' + chr(-self.maxSpeed + 255) + chr(-self.maxSpeed + 255))
		elif key == ord('1'):
			self.commThread.write('u')
		elif key == ord('2'):
			self.commThread.write('d')
		elif key == ord('q') or key == 32:
			self.commThread.write('s' + chr(0) + chr(0))
		elif key != 255:
			print key

		return False

	def stateSearching(self):
		data = self.cameraThread.checkDataQueue()
		if data:
			self.commThread.write("!" + chr(data["position"][0] + chr(data["position"][1]))
			print "Bottle detected", data["position"]

robot = RobotControl("/dev/ttyACM0", 9600)
robot.connect()
