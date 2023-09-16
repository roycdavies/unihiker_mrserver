# -*- coding: utf-8 -*-
import time
from pinpong.board import *
from pinpong.extension.unihiker import *
import math
from unihiker import GUI

direction = 0

gui = GUI()
accelx_gui = gui.draw_text(x=120, y=0, text="", origin="n", font_size=12)
accely_gui = gui.draw_text(x=120, y=15, text="", origin="n", font_size=12)
accelz_gui = gui.draw_text(x=120, y=30, text="", origin="n", font_size=12)

gyrox_gui = gui.draw_text(x=120, y=60, text="", origin="n", font_size=12)
gyroy_gui = gui.draw_text(x=120, y=75, text="", origin="n", font_size=12)
gyroz_gui = gui.draw_text(x=120, y=90, text="", origin="n", font_size=12)

leftorright = gui.draw_text(x=120, y=120, text="", origin="n", font_size=12)


Board().begin()  # Initialize the UNIHIKER

while True:
    # print("Accel X", round(accelerometer.get_x(),2))  # Read the value of acceleration in the X-axis
    accelx_gui.config(text="Accel X " + str(round(accelerometer.get_x(),2)))
    accely_gui.config(text="Accel Y " + str(round(accelerometer.get_y(),2)))
    accelz_gui.config(text="Accel Z " + str(round(accelerometer.get_z(),2)))
    
    gyrox_gui.config(text="Gyro X " + str(round(gyroscope.get_x(),2)))
    gyroy_gui.config(text="Gyro Y " + str(round(gyroscope.get_y(),2)))
    gyroz_gui.config(text="Gyro Z " + str(round(gyroscope.get_z(),2)))
    
    if (gyroscope.get_z() > 0.1):
        direction -= 1
        if (direction < -3):
            direction = -3
    elif(gyroscope.get_z() < -0.1):
        direction += 1
        if (direction > 3):
            direction = 3
        
    if (direction > 2):
        leftorright.config(text="RIGHT")
    elif(direction < -2):
        leftorright.config(text="LEFT")
    else:
        leftorright.config(text="Straight")
    # print("Accel Y", round(accelerometer.get_y(),2))  # Read the value of acceleration in the Y-axis
    # print("Accel Z", round(accelerometer.get_z(),2))  # Read the value of acceleration in the Z-axis
    # print("Strength", round(accelerometer.get_strength(),2))  # Read the total strength of acceleration (combination of X, Y, and Z axes)
    # print("Gyro X", round(gyroscope.get_x(),2))  # Read the value of gyroscope in the X-axis
    # print("Gyro Y", round(gyroscope.get_y(),2))  # Read the value of gyroscope in the Y-axis
    # print("Gyro Z", round(gyroscope.get_z(),2))  # Read the value of gyroscope in the Z-axis
    # print("------------------")
    time.sleep(0.1)