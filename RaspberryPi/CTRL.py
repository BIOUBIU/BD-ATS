#! /usr/bin/env python3
# -*- coding: utf-8 -*
import serial
import time
import RPi.GPIO as GPIO
import os
import WBFM
import NBFM

serCmd = serial.Serial("/dev/ttyAMA1", 9600, timeout = 10)
serUSB = serial.Serial("/dev/ttyUSB0", 9600, timeout = 10)#######################

buf = 'buf'
bufList = ['0']

def setupSerial():
    global serCmd#, serUSB
    while not serCmd.is_open():
        pass  
    while not serUSB.is_open():
        pass
    return

##############################
def setupPredict():
    return

def taskArrange(satName, mode, freq, startTime, endTime):
    return

def tleStorage(satName, tleLn1, tleLn2):
    return

def timeCorrection(time):
    cmd = "date " + time
    os.system(cmd)
    return

def info():#######################
    return

def superdo(command):
    os.system(command)
    return

"""
    @描述:通过REP语句向北斗上位机请求任务语句
"""
def REPRequest():
    global serCmd, buf, bufList, serUSB
    serCmd.write(b"REP,timeRequest\n")
    if(serCmd.in_waiting() != 0):
        buf = serCmd.read_until()
        bufList = buf.split(',')
        if(bufList[0] == 'REP'):
            timeCorrection(bufList[1])
    ##############################################
    serCmd.write(b"REP,taskRequest\n")
    if(serCmd.in_waiting() != 0):
        buf = serCmd.read_until()
        bufList = buf.split(',')
        if(bufList[0] == 'TSK'):
            taskArrange(bufList[1], bufList[2], bufList[3], bufList[4], bufList[5])#1：名称 2：调制 3：边带 4：中心频点（Hz） 5：唤醒
    return

def destory():
    serCmd.close()
    serUSB.close()
    GPIO.cleanup()
    return

if __name__ == '__main__':
    try:
        setupSerial()
        REPRequest()
        while True:
            if (serCmd.in_waiting() != 0):
                buf = serCmd.read_until()
                bufList = buf.split(',')
                if(bufList[0] == 'TSK'):
                    taskArrange(bufList[1], bufList[2], bufList[3], bufList[4])
                elif(bufList[0] == 'TLE'):
                    tleStorage(bufList[1], bufList[2], bufList[3])
                #elif(bufList[0] == 'INF'):
                #    info()
                elif(bufList[0] == 'SUP'):
                    superdo(bufList[1])
                else:
                    raise

    except Exception:
        serCmd.write(b"MSG,An error has occured in CTRL.py")
        destory()

