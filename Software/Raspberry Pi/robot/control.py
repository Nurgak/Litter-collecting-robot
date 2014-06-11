#!/usr/bin/env python
import time
import sys
from cv2 import waitKey

import comm
import camera

# the available states in which the robot can be at any moment (emulate enum)
SEARCHING, GRABBING, HOMING, RELEASE = range(4)

# state machine return codes
FAIL, OK, REPEAT = range(3)

# types of elements to detect
OBSTACLE, BOTTLE = range(2)

class RobotControl:
	# definitions on how to transition from one state to another according to return codes
	stateTransitions = [
		{"state": SEARCHING,	"code": OK,		"next": GRABBING},
		{"state": SEARCHING,	"code": REPEAT,	"next": SEARCHING},
		{"state": GRABBING,		"code": FAIL,	"next": SEARCHING},
		{"state": GRABBING,		"code": OK,		"next": HOMING},
		{"state": GRABBING,		"code": REPEAT,	"next": GRABBING},
		{"state": HOMING,		"code": OK,		"next": RELEASE},
		{"state": HOMING,		"code": FAIL,	"next": RELEASE},
		{"state": HOMING,		"code": REPEAT,	"next": HOMING},
		{"state": RELEASE,		"code": OK,		"next": SEARCHING}
	]

	def __init__(self, port, baudrate):
		# configuration for the serial connection to the robot
		self.port = port
		self.baudrate = baudrate

		self.maxSpeed = 70

		self.currentState = SEARCHING
		self.returnCode = REPEAT
		
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
		
		while True:
			try:
				# run the finite state machine
				for transition in self.stateTransitions:
					if self.currentState == transition["state"] and self.returnCode == transition["code"]:
						# set new state
						self.currentState = transition["next"]

						# actually execute the state method
						if self.currentState == SEARCHING:
							self.returnCode = self.stateSearching()
						elif self.currentState == GRABBING:
							self.returnCode = self.stateGrabbing()
						elif self.currentState == HOMING:
							self.returnCode = self.stateHoming()
						elif self.currentState == RELEASE:
							self.returnCode = self.stateRelease()

						# break the inner state machine loop and start over
						break

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
		#print "Searching state"

		data = self.cameraThread.checkDataQueue()

		if data:
			if data["type"] == OBSTACLE:
				print "Obstacle detected", data["position"]

			if data["type"] == BOTTLE:
				print "Bottle detected", data["position"]

		'''# obstacle avoidance
		if (obstacle = self.cameraThread.checkObstacle()):
			print "Obstacle detected: ", obstacles
			# check immediately in front of the robot
			if obstacles[1] > self.obstacleThreshold:
				self.commThread.setSpeed(self.maxSpeed, -self.maxSpeed)
			elif obstacles[2] > self.obstacleThreshold:
				self.commThread.setSpeed(-self.maxSpeed, self.maxSpeed)
			# check on the sides
			elif obstacles[0] > self.obstacleThreshold:
				self.commThread.setSpeed(0, self.maxSpeed)
			elif obstacles[3] > self.obstacleThreshold:
				self.commThread.setSpeed(self.maxSpeed, 0)
			else:
				self.commThread.setSpeed(self.maxSpeed, self.maxSpeed)
		# check for bottle when no obstacle is in sight
		elif (bottle = self.cameraThread.detectBottles()):
			print "Bottle detected"
			# a bottle has been found, go to the next state
			return OK'''

		# as long as there are no bottles in sight continue roaming
		return REPEAT

	def stateGrabbing(self):
		print "Grabbing state"
		# approach bottle carefully and lower the cage

		return OK

	def stateHoming(self):
		print "Homing state"
		# move towards the home corner and make sure we're at home before passing to the next stage

		return OK

	def stateRelease(self):
		print "Release state"
		# raise the cage

		return OK

robot = RobotControl("/dev/ttyACM0", 9600)
robot.connect()
