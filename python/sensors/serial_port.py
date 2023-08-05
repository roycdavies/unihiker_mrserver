# -*- coding: utf-8 -*-
import time
from pinpong.board import Board
from pinpong.board import UART

Board("UNIHIKER").begin()  # Initialize the UNIHIKER board. Select the board type, if not specified, it will be automatically detected.

#hardware UART 1 P0-RX P3-TX
uart1 = UART()   # Initialize UART (Hardware Serial 1)
uart1.init()  # Initialize UART with default baud rate (9600)

buf = [0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x07]

# Close hardware UART 1
uart1.deinit() 

# Get the number of bytes available to read from UART
uart1.any() 

# Write data to the UART. buf is a list of bytes.
uart1.write(buf)

# Read characters from the UART. Returns None or a list of bytes.
uart1.read(n)

# Read a line until a newline character is received. Reads until newline or None if timeout occurs.
buf = uart1.readline()

# Read bytes into buf. If nbytes is specified, read at most that many bytes. Otherwise, read at most len(buf) bytes.
uart1.readinto(buf, nbytes)

while True:
    uart1.write(buf)  # Write data to UART
    time.sleep(1)