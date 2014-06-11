#!/usr/bin/env python
import sys
import serial
import threading
import time
import socket

class CommThread(threading.Thread):
    def __init__(self, port, baudrate):
        # script crashes without this
        super(CommThread, self).__init__()
        # save handle
        self.comm = serial.Serial(port=port, baudrate=baudrate, timeout=1)
        # an event that can be used for anything, here used to stop the thread
        self.running = threading.Event()

    def write(self, data):
        # send data to the device
        try:
            return self.comm.write(data)
        except:
            return False

    def setSpeed(self, speedLeft, speedRight):
        # must be unsigned value
        if speedLeft < 0:
            speedLeft += 255

        if speedRight < 0:
            speedRight += 255

        return self.write('s' + chr(speedLeft) + chr(speedRight))

    def run(self):
        print "Communication thread started"
        # make sure we're still running
        while not self.running.isSet():
            # see if there is something in the incoming buffer
            if self.comm.inWaiting():
                self.process(self.comm.readline())
            
            # wait for a bit or it will consume all the CPU
            time.sleep(0.2)

    def process(self, data):
        if data == 'p':
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            # create a socket to an external website
            s.connect(('google.com', 0))
            ip = s.getsockname()[0].split(".")
            # transform list of strings to list of integers
            ip = map(int, ip)
            # send the external ip address via serial to the arduino board
            self.write(chr(ip[0]) + chr(ip[1]) + chr(ip[2]) + chr(ip[3]))

    def stop(self):
        print "Stopping communication thread"
        self.running.set()
