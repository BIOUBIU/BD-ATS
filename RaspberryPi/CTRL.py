#! /usr/bin/env python3
# -*- coding: utf-8 -*
import serial
import time
import RPi.GPIO as GPIO

serCmd = serial.Serial("/dev/ttyAMA1", 9600, timeout = 10)
serUSB = serial.Serial("/dev/ttyUSB0", 9600, timeout = 10)#######################

buf = 'buf'
bufList = ['0']

def setupSerial():
    global serCmd, serUSB
    while not serCmd.is_open():
        pass
    while not serUSB.is_open():
        pass
    return

##############################
def setupGPredict():
    return

def taskArrange(satName, mode, freq, time):
    return

"""
    @描述:通过REP语句向北斗上位机请求任务语句
"""
def REPTaskRequest():
    global serCmd, serUSB, buf, bufList
    serCmd.write(b"REP,taskRequest\n")
    if(serCmd.inWaiting() != 0):
        buf = serCmd.read_until()
        bufList = buf.split(',')
        if(bufList[0] == 'TSK'):
            taskArrange(bufList[1], bufList[2], bufList[3], bufList[4])
    return

def destory():
    GPIO.cleanup()
    return

if __name__ == '__main__':
    try:
        setupSerial()
        #REPTaskRequest()
        
    except KeyboardInterrupt:
        destory()

