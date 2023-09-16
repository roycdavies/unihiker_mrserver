import numpy as np
import time
from pinpong.board import *
from pinpong.extension.unihiker import *
import math

roll = 0
pitch = 0
yaw = 0

def calculate(roll, pitch, yaw, dr, dp, dy, dt):
    return roll + dr * dt, pitch + dp * dt, yaw + dy * dt

def calculate_orientation(accel_data, gyro_data, yaw, dt):
    # Extract accelerometer data
    ax, ay, az = accel_data

    # Extract gyroscope data
    gx, gy, gz = gyro_data

    # Calculate roll and pitch angles from accelerometer data
    roll = math.atan2(ay, az)
    pitch = math.atan2(-ax, math.sqrt(ay**2 + az**2))

    # Integrate gyroscope data to update roll and pitch
    roll += gx * dt
    pitch += gy * dt

    # Calculate yaw (heading) from gyroscope data
    yaw = yaw/10 + gz * dt * az

    return roll, pitch, yaw*10



def complementary_filter(acceleration, gyro_rate, dt, alpha):
    """
    Complementary Filter to estimate roll and pitch angles.
    - acceleration: 3-element array [ax, ay, az] from accelerometer (m/s^2)
    - gyro_rate: 3-element array [gx, gy, gz] from gyroscope (rad/s)
    - dt: time step (seconds)
    - alpha: complementary filter constant (0 < alpha < 1)
    """
    acc_pitch = np.arctan2(acceleration[1], np.sqrt(acceleration[0]**2 + acceleration[2]**2))
    gyro_pitch = acc_pitch + gyro_rate[0] * dt

    acc_roll = np.arctan2(-acceleration[0], np.sqrt(acceleration[1]**2 + acceleration[2]**2))
    gyro_roll = acc_roll + gyro_rate[1] * dt

    return (alpha * gyro_roll) + ((1 - alpha) * acc_roll), (alpha * gyro_pitch) + ((1 - alpha) * acc_pitch)

# Example data
# acceleration = np.array([0.1, 0.2, 9.8])  # m/s^2
# gyro_rate = np.array([0.02, 0.01, 0.03])  # rad/s
dt = 0.1  # Time step (10 ms)
alpha = 0.98  # Complementary filter constant

Board().begin()  # Initialize the UNIHIKER

while True:
    acceleration = [accelerometer.get_x(), accelerometer.get_y(), accelerometer.get_z()]
    gyro_rate = [gyroscope.get_x(), gyroscope.get_y(), gyroscope.get_z()]
    roll, pitch, yaw = calculate_orientation(acceleration, gyro_rate, yaw, dt)
    print("Roll (degrees):", math.degrees(roll))
    print("Pitch (degrees):", math.degrees(pitch))
    print("Yaw (degrees):", math.degrees(yaw))
    time.sleep(0.1)
