import serial.tools.list_ports as port_list
import serial
import time

# https://pypi.org/project/pyubx2/
from pyubx2 import UBXReader

ports = list(port_list.comports())
for p in ports:
    print (p)
    
s = serial.Serial('/dev/ttyACM0', 9600)
ubr = UBXReader(s)

while True:
    (raw_data, parsed_data) = ubr.read()
    print(parsed_data)

    # res = s.readline()
    # print(res)
    # time.sleep(0.1)