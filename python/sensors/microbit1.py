from pinpong.board import Board, Pin
import time

Board().begin()

led = Pin(Pin.P15, Pin.OUT)

while True:
    led.write_digital(1)
    time.sleep(1)
    led.write_digital(0)
    time.sleep(1)
