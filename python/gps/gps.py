import serial.tools.list_ports as port_list
import serial
import time
import re
import pygeohash

prev_geohash = ""

def extract_lat_lon(input_str):
    pattern = r'lat=(-?\d+\.\d+),\s*NS=([NS]),\s*lon=(-?\d+\.\d+),\s*EW=([EW])'
    match = re.search(pattern, input_str)

    if match:
        latitude = float(match.group(1))
        longitude = float(match.group(3))

        return (latitude, longitude)

    return (None, None)  # Return None if the input doesn't match the expected format


# https://pypi.org/project/pyubx2/
from pyubx2 import UBXReader

ports = list(port_list.comports())
for p in ports:
    print (p)
    
s = serial.Serial('/dev/ttyACM1', 9600)
ubr = UBXReader(s)

while True:
    (raw_data, parsed_data) = ubr.read()
    
    print(str(parsed_data))
    (lat, lon) = extract_lat_lon(str(parsed_data))
    if (lat != None):
        geohash = pygeohash.encode(lat, lon)
        if (geohash != prev_geohash):
            print(lat, lon, geohash)
            prev_geohash = geohash