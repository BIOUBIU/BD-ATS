#! /usr/bin/env python3
# -*- coding: utf-8 -*
import serial
import time
import datetime
import RPi.GPIO as GPIO
import os
import WBFM
import NBFM
import subprocess
import logging
import numpy as np

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
    logging.info('UART setup')
    return

#创建多普勒文件，用以修正多普勒，从predict的stdout中读数据并写入一个txt,返回开始的unix时间戳用于输给GR
def createDoppler(satName, freq, startTime):
    #os.remove("doppler.txt")
    logging.info('generating doppler file for %s working at %s Hz after %s'%(satName,freq,startTime))
    cmd = "predict -dp " + satName + startTime
    doppler = subprocess.Popen(args = cmd, shell = True, stdout = subprocess.PIPE)
    logging.info('create doppler subprocess')
    dpout = doppler.stdout.readlines
    doppler.kill()
    logging.info('kill doppler subprocess')
    flag = 0
    unixStartTime = 0
    with open('doppler.txt','w+',encoding='utf-8') as dp:
        logging.info('writing doppler.txt')
        for line in dpout:
            word = line.split(',')
            timeStamp = word[0]
            if(flag == 0):
                unixStartTime = timeStamp
                flag = 1
                logging.info('unixStartTime generated, turn flag into 1')
            shiftInt = int(word[2])
            freqInt = int(freq)
            dopplerInt = shiftInt + freqInt
            dopplerStr = str(dopplerInt)
            oneLine = timeStamp + ' ' + dopplerStr
            dp.write(oneLine)
    logging.info("%s's doppler.txt generated" % satName)
    return unixStartTime

def telemetryDecode(satName, startTime):
    cmd = 'gr_satellites ' +  satName + ' --wavfile ~/radio/recording/recording.wav'
    grsat = subprocess.Popen(args = cmd, shell = True, stdin = subprocess.PIPE, stdout = subprocess.PIPE)
    grsat.wait()
    out = grsat.stdout.read()
    fileName = startTime + '.txt'
    with open(fileName,'w+',encoding='utf-8') as tm:
        tm.write(out)
    summary = 'MSG,3,' + out[0:out.find('ListContainer')] + '$'
    serCmd.write(summary)
    grsat.kill()
    return

def task(satName, mode, sideband, freq, startTime, endTime):
    logging.info('task started:%s %s %s %s %s %s'%(satName,mode,sideband,freq,startTime,endTime))
    st = datetime.datetime.strptime(startTime, "%Y-%m-%d %H:%M:%S")
    unixST = st.timestamp()
    et = datetime.datetime.strptime(endTime, "%Y-%m-%d %H:%M:%S")
    unixET = et.timestamp()
    unixSTstr = str(unixST)
    unixST4dp = createDoppler(satName, freq, unixSTstr) #给多普勒修正用的unix开始时间戳
    cmd1 = "predict -a /dev/ttyAMA2"
    predict = subprocess.Popen(args = cmd1, shell = True,stdin = subprocess.PIPE, stdout = subprocess.PIPE)
    logging.info('run predict:%s' % cmd1)
    predict.wait(5)
    predict.communicate('T')
    satlist = np.load('~/radio/SatList.npy').item()
    logging.info('satlist loaded')
    predict.communicate(satlist[satName])
    while(time.time() <= unixST):
        time.sleep(0.5)
        pass
    if(mode == 'NBFM'):
        NBFM.main(endTime=unixET,freq=int(freq),doppler_start_time=int(unixST4dp))
    if(mode == 'WBFM'):
        WBFM.main()
    predict.kill()
    telemetryDecode(satName, startTime)
    return

def tleStorage(satName, tleLn1, tleLn2):
    with open('tle.txt','w+',encoding='utf-8') as tle:
        tle.write(satName + '\n')
        tle.write(tleLn1 + '\n')
        tle.write(tleLn2 + '\n')
    predict = subprocess.Popen(args = 'predict', shell = True,stdin = subprocess.PIPE, stdout = subprocess.PIPE)
    predict.communicate('U')
    predict.wait(2)
    predict.communicate('~/radio/tle.txt')
    predict.wait(2)
    predict.kill()
    return

def timeCorrection(time):
    cmd = "date -s '%s'" % (time)
    os.system(cmd)
    os.system('hwclock -w')
    return


def info():
    with open('last.log','r',encoding='utf-8') as inf:
        log = inf.read()
    log = 'MSG,2,' + log + '$'
    serCmd.write(log)
    return

def superdo(command):
    os.system(command)
    return

"""
    @描述:通过REP语句向北斗上位机请求任务语句
"""
def REPRequest():
    global serCmd, buf, bufList, serUSB
    serCmd.write("REP,timeRequest\n")   #b前缀去掉了
    while(serCmd.in_waiting() == 0):
        pass
    buf = serCmd.read_until()
    bufList = buf.split(',')
    if(bufList[0] == 'REP'):
        timeCorrection(bufList[1])
    else:
        raise
    serCmd.write("REP,taskRequest\n")
    while(serCmd.in_waiting() == 0):
        pass
    buf = serCmd.read_until()
    bufList = buf.split(',')
    if(bufList[0] == 'TSK'):
        task(bufList[1], bufList[2], bufList[3], bufList[4], bufList[5], bufList[6])#1：名称 2：调制 3：边带 4：中心频点（Hz） 5：唤醒 6:结束时间    
    elif(bufList[0] == 'INF'):
        info()
    else:
        raise
    return

def destory():
    serCmd.close()
    serUSB.close()
    GPIO.cleanup()
    return

if __name__ == '__main__':
    try:
        logging.basicConfig(filename="last.log", filemode="w+", format="%(asctime)s %(name)s:%(levelname)s:%(message)s", datefmt="%d-%M-%Y %H:%M:%S", level=logging.DEBUG)
        setupSerial()
        REPRequest()
        while True:
            if (serCmd.in_waiting() != 0):
                buf = serCmd.read_until()
                bufList = buf.split(',')
                if(bufList[0] == 'TSK'):
                    task(bufList[1], bufList[2], bufList[3], bufList[4], bufList[5], bufList[6])
                elif(bufList[0] == 'TLE'):
                    tleStorage(bufList[1], bufList[2], bufList[3])
                elif(bufList[0] == 'INF'):
                    info()
                elif(bufList[0] == 'SUP'):
                    superdo(bufList[1])
                else:
                    raise

    except Exception:
        serCmd.write("MSG,A FATAL error has occured in CTRL.py, please restart Pi through ESP32")
        destory()
        os.system('shutdown')

