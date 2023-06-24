#! /usr/bin/env python3
# -*- coding: utf-8 -*
import serial
import time
import RPi.GPIO as GPIO

serCmd = serial.Serial("/dev/ttyAMA1",9600)

def setupSerial():
    global serCmd
    while not serCmd.is_open():
        pass
    return

##############################
def setupGPredict():
    return

"""
    @描述:通过REP语句向北斗上位机请求任务语句
"""
def REPTaskRequest():
    global serCmd
    serCmd.write(b"REP,taskRequest\n")
