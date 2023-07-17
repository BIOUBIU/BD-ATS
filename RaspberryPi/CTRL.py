#! /usr/bin/env python3
# -*- coding: utf-8 -*
import serial
import time
import RPi.GPIO as GPIO
import os
import WBFM
import NBFM
import subprocess


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

#创建多普勒文件，用以修正多普勒，从predict的stdout中读数据并写入一个txt
def createDoppler(satName, freq, startTime):
    #os.remove("doppler.txt")
    cmd = "predict -dp " + satName + startTime
    doppler = subprocess.Popen(args = cmd, shell = True, stdout = subprocess.PIPE)
    dpout = doppler.stdout.readlines
    doppler.kill()
    flag = 0
    unixStartTime = 0
    with open('doppler.txt','w+',encoding='utf-8') as dp:
        for line in dpout:
            word = line.split(',')
            timeStamp = word[0]
            if(flag == 0):
                unixStartTime = timeStamp
                flag = 1
            shiftInt = int(word[2])
            freqInt = int(freq)
            dopplerInt = shiftInt + freqInt
            dopplerStr = str(dopplerInt)
            oneLine = timeStamp + ' ' + dopplerStr
            dp.write(oneLine)
    return unixStartTime
    
def taskArrange(satName, mode, freq, startTime, endTime):
    cmd1 = "predict -a /dev/ttyAMA2"
    predict = subprocess.Popen(args = cmd1, shell = True,stdin = subprocess.PIPE, stdout = subprocess.PIPE)
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
            taskArrange(bufList[1], bufList[2], bufList[3], bufList[4], bufList[5], bufList[6])#1：名称 2：调制 3：边带 4：中心频点（Hz） 5：唤醒 6:结束时间    
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
                    taskArrange(bufList[1], bufList[2], bufList[3], bufList[4], bufList[5], bufList[6])
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

