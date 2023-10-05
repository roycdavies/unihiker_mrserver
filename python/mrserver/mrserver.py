# ****************************************************************************************************
# Roy C. Davies, 2023
# ****************************************************************************************************
from unihiker import GUI
import time
from pinpong.board import *
from pinpong.extension.unihiker import *
import subprocess
import threading
import ifaddr
import ssl
import websocket
import serial.tools.list_ports as port_list
import serial
import time
import re
import pygeohash
from pyubx2 import UBXReader    # https://pypi.org/project/pyubx2/

from api import *
from fsm import *

# ====================================================================================================
# Variables
# ====================================================================================================
mrserver = "/root/cloud-mixed-reality-server-erlang/MRServer/mrserver/_rel/mrserver/bin/mrserver"
unihiker_config_file = "/opt/unihiker/pyboardUI/config.cfg"
unihiker_config = {}
useremail = "roy@imersia.com"
password = "1gelk0tt"
led = ""
rgblight = ""
ipaddr = ""
something_changed = False
running = True
lighton = True
messageGUI = ""
stateGUI = ""
gpsGUI = ""
qrcode = ""
qrcode_text = ""
geobotid1 = "a10ba25c-e8b1-11ea-b1e6-6474f696ef8e"
geobotid2 = "8de3d445-112f-11ee-913c-7cf01f83542a"
trackedgeobot = "92ffcbea-54c6-11ee-ad48-1a9b443f7516"
gps_serial_port = None
prev_geohash = ""
server = {
    "apiurl": "https://localhost/api",
    "sessionid": "",
    "userid": "9527ca9c-e8b1-11ea-9b9c-647413e74bfc",
    "developerid": "test",
    "geohash": "rcknm16mwdqz"
}

display_wifi_qr = True

# WIFI Hotspot
wifi_ssid = ""
wifi_password = ""

# Set the GUI
gui = GUI()
stateGUI = gui.draw_text(x=120, y=0, text="", origin="n", font_size=10)
gpsGUI = gui.draw_text(x=120, y=20, text="", origin="n", font_size=10)

ssl_context = ssl.SSLContext()
ssl_context.verify_mode = ssl.CERT_NONE


# ====================================================================================================


# ====================================================================================================
# Create the FSM
# ====================================================================================================
MRServer = Automation() 
# ====================================================================================================


# ====================================================================================================
# Service functions
# ====================================================================================================


# ----------------------------------------------------------------------------------------------------
# Extract Latitude and Longitude from the GPS string
# ----------------------------------------------------------------------------------------------------
def extract_lat_lon(input_str):
    pattern = r'lat=(-?\d+\.\d+),\s*NS=([NS]),\s*lon=(-?\d+\.\d+),\s*EW=([EW])'
    match = re.search(pattern, input_str)

    if match:
        latitude = float(match.group(1))
        longitude = float(match.group(3))

        return (latitude, longitude)

    return (None, None)  # Return None if the input doesn't match the expected format
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# Determine if the serial port has a GPS unit, and if so, what port it is on
# ----------------------------------------------------------------------------------------------------
def extract_serial_port(ports):
    print ("Serial Ports:")
    for p in ports:
        print(p)
        # Check if the port contains the keyword "GPS"
        if "GPS" in str(p):
            # Use regular expression to extract the serial port information
            serial_port_match = re.search(r'/dev/tty\w+', str(p))
            if serial_port_match:
                return serial_port_match.group()
    
    return None  # Return None if it's not a GPS unit or no serial port found
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# Generate a QR Code string for the hotspot
# ----------------------------------------------------------------------------------------------------
def generate_wifi_qr_code(ssid, password, authentication_type='WPA'):
    wifi_string = f'WIFI:T:{authentication_type};S:{ssid};P:{password};;'
    return wifi_string
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# Button clicks
# ----------------------------------------------------------------------------------------------------
def on_a_click():
    global MRServer
    MRServer.event("a_button")
def on_b_click():
    global MRServer
    MRServer.event("b_button")
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------------
def print_state():
    global stateGUI, MRServer, running
    
    while running:
        stateGUI.config(text="state = " + MRServer._state)
        time.sleep(0.05)
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------------
def get_hotspot_ip():
    ipaddr = ""
    adapters = ifaddr.get_adapters()
    # print (adapters)
    
    # First see if there is a wifi connection
    for adapter in adapters:
        if "wlan0" in adapter.nice_name:
            for ip in adapter.ips:
                if ip.is_IPv4:
                    ipaddr = ip.ip
                    
    # Otherwise see if there is a hotspot
    if (ipaddr == ""):
        for adapter in adapters:
            if "p2p0" in adapter.nice_name:
                for ip in adapter.ips:
                    if ip.is_IPv4:
                        ipaddr = ip.ip
                        
    return ipaddr
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# Switch between QR codes
# ----------------------------------------------------------------------------------------------------
def switch_qr():
    global display_wifi_qr, qrcode, qrcode_text, ipaddr, wifi_ssid, wifi_password, geobotid1, geobotid2
    
    display_wifi_qr = not display_wifi_qr
    
    if display_wifi_qr:
        qrcode.config(text=generate_wifi_qr_code(wifi_ssid, wifi_password))
        qrcode_text.config(text="ssid: " + wifi_ssid + " pass: " + wifi_password)
    else:
        qrcode.config(text="https://" + ipaddr + "/?id=" + geobotid1)
        qrcode_text.config(text="geobot")
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# Set things up
# ----------------------------------------------------------------------------------------------------
def initialise():
    global messageGUI, stateGUI, requests, gui, qrcode, qrcode_text, display_wifi_qr, led, rgblight, wifi_ssid, wifi_password, ipaddr
    
    with open(unihiker_config_file) as user_file:
        unihiker_config = json.loads(user_file.read())
        wifi_ssid = unihiker_config["apName"]
        wifi_password = unihiker_config["apPassword"]
    
    # Get the IP Addr of this MRServer
    ipaddr = get_hotspot_ip()

    # Setup the callbacks for the buttons
    gui.on_a_click(on_a_click)
    gui.on_b_click(on_b_click)

    # Draw the original message and keep the GUI object
    messageGUI = gui.draw_text(x=120, y=50, text="Initialising", origin="n", font_size=10)
    
    # Create a scannable QR code to the Geobot
    if (ipaddr == ""):
        gui.draw_text(x=120, y=170, text="No Network", origin="n", font_size=10)
    else:
        qrcode = gui.draw_qr_code(x=120, y=170, w=180, text=generate_wifi_qr_code(wifi_ssid, wifi_password), origin="center")
        qrcode_text = gui.draw_text(x=120, y=250, text="ssid: " + wifi_ssid + " pass: " + wifi_password, origin="n", font_size=10)
    
    # Interaction Buttons
    gui.add_button(x=120, y=290, w=100, h=30, text="Toggle", origin="center", onclick=switch_qr)

    # Show the state of the FSM
    gui.start_thread(print_state)

    # Disable warnings due to not having proper certificates for SSH
    requests.packages.urllib3.disable_warnings()

    # Start the sensors board
    Board().begin()
    led = Pin(Pin.P25, Pin.OUT)
    rgblight = Pin(Pin.P15, Pin.OUT)
# ----------------------------------------------------------------------------------------------------

# ====================================================================================================



# ====================================================================================================
# Actions
# ====================================================================================================

# ----------------------------------------------------------------------------------------------------
# Check if MRServer is running
# ----------------------------------------------------------------------------------------------------
def check_mrserver(_):
    processes = subprocess.run(["ps", "-e"], capture_output=True)
    return ("beam.smp" in str(processes.stdout))
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------------
def check_receiving(_):
    global server
    
    data = OPTIONS(server, "sessions", {})
    if data["status"] != -500:
        MRServer.event("serverok")
        return (True)
    else:
        MRServer.event("checkserver", 2)
        return (False)
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# Start MRServer
# ----------------------------------------------------------------------------------------------------
def start_thread():
    subprocess.run([mrserver, "daemon"])

def start_mrserver(is_running):
    global MRServer
    if (not is_running):
        the_server = threading.Thread(target=start_thread)
        the_server.start()
        
    MRServer.event("checkserver", 1)

    return (True)
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# Stop MRServer
# ----------------------------------------------------------------------------------------------------
def stop_thread():
    subprocess.run(["killall", "heart"])
    subprocess.run(["killall", "mrserver"])

def stop_mrserver(is_running):
    global MRServer
    if (is_running):
        the_server = threading.Thread(target=stop_thread)
        the_server.start()

    return (True)
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# Log in
# ----------------------------------------------------------------------------------------------------
def log_in(_):
    global useremail, password, MRServer, server

    # Log in
    data = GET(server, "sessions", {"useremail":useremail, "password":password})
    if data["status"] == 200:
        print(data["result"])
        server["sessionid"] = data["result"]["sessionid"]
        MRServer.event("loggedin")
        return (True)
    else:
        MRServer.event("login", 2)
        return (False)
# ----------------------------------------------------------------------------------------------------


# ----------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------------
def send_event(event):
    global server

    # Send an event to a geobote
    POST(server, "geobots/send", {"geobotid": geobotid1}, {"event":event, "parameters":{}})
# ----------------------------------------------------------------------------------------------------



def set_geobot_position(geobotid, geohash):
    global server
    
    PUT(server, "geobots", {"geobotid": geobotid}, {"location": {"geohash":geohash}})



# ----------------------------------------------------------------------------------------------------
# set the LED
# ----------------------------------------------------------------------------------------------------
def set_led(value):
    global led, rgblight
    led.write_digital(int(value))
    # rgblight.write_digital(int(value))
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# Interpret a message from the websocket companion
# ----------------------------------------------------------------------------------------------------
def interpret_message(message):
    messagejson = json.loads(message)
    if ("wotcha" in messagejson):
        wotcha = messagejson["wotcha"]
        # print (wotcha)
        if ("trigger" in wotcha):
            trigger = wotcha["trigger"]
            # print (trigger)
            if not ("channelid" in trigger):
                print (trigger["event"])
                MRServer.event(trigger["event"])
# ----------------------------------------------------------------------------------------------------
                
                

# ----------------------------------------------------------------------------------------------------
# Connect to the companion, wotch the given Geobot, and respond appropriately to messages
# ----------------------------------------------------------------------------------------------------
def wait_for_message(ws):
    global running
    while running:
        message = ws.recv()
        if (message == "ping"):
            ws.send("pong")
            print ("WebSocket Live")
        else:
            interpret_message(message)
        # print(f"Received: {message}")
    ws.close()
        
def connect():
    global userid, ssl_context, server, geobotid2, running
    
    ws = websocket.WebSocket(sslopt={"cert_reqs": ssl.CERT_NONE})
    ws.connect("wss://localhost/wotcha/" + server["userid"])
    command = {
        "command": "wotcha",
        "sessionid": server["sessionid"],
        "parameters": {
            "id": geobotid2
        }
    }
    ws.send(json.dumps(command))
    print("Connected")
    MRServer.event("connected")

    the_companion = threading.Thread(target=wait_for_message, args=(ws,))
    the_companion.start()
        
def connect_to_companion(_):
    start_websocket = threading.Thread(target=connect)
    start_websocket.start()
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# Change the message on the screen
# ----------------------------------------------------------------------------------------------------
def printout(something):
    global messageGUI, MRServer

    messageGUI.config(text=something)
    MRServer.event("clear", 1)
    
    print (something)
    return True
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------------
def clear(_):
    global messageGUI
    messageGUI.config(text="")
    return True
# ----------------------------------------------------------------------------------------------------



# ----------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------------
def quit(_):
    global running

    running = False
# ----------------------------------------------------------------------------------------------------

# ====================================================================================================


# ====================================================================================================
# Main Code
# ====================================================================================================

# ----------------------------------------------------------------------------------------------------
# Set the transitions
# ----------------------------------------------------------------------------------------------------
#                       state               event               newstate            actions
MRServer.add(Transition("start",            "init",             "starting",         ["Starting MRServer", printout, check_mrserver, start_mrserver]))

MRServer.add(Transition("starting",         "checkserver",      "starting",         [check_receiving]))
MRServer.add(Transition("starting",         "serverok",         "loggingin",        ["Logging in", printout, log_in]))

MRServer.add(Transition("loggingin",        "login",            "loggingin",        [log_in]))
MRServer.add(Transition("loggingin",        "loggedin",         "wotching",         ["MRServer ready", printout, connect_to_companion]))
MRServer.add(Transition("loggingin",        "error",            "error",            ["Error", printout]))

MRServer.add(Transition("wotching",         "connected",        "companion",        ["Connected to Companion", printout]))
MRServer.add(Transition("wotching",         "error",            "error",            ["Error", printout]))

MRServer.add(Transition("companion",        "error",            "error",            ["Error", printout]))


MRServer.add(Transition("companion",        "a_button",         "companion",        ["Sending Message", printout, "touch", send_event]))
MRServer.add(Transition("companion",        "on",               "companion",        ["ON", printout, "1", set_led]))
MRServer.add(Transition("companion",        "off",              "companion",        ["OFF", printout, "0", set_led]))

MRServer.add(Transition("*",                "clear",            "*",                [clear]))
MRServer.add(Transition("*",                "b_button",         "quitting",         ["Quitting...", printout, check_mrserver, stop_mrserver, quit]))
# ----------------------------------------------------------------------------------------------------

# Open the serial port for the GPS unit
gps_serial_port = extract_serial_port(list(port_list.comports()))

if (gps_serial_port != None):  
    s = serial.Serial(gps_serial_port, 9600)
    ubr = UBXReader(s)

# Initialise various things
initialise()

# Set the whole thing going
print ("Starting MRServer")
MRServer.go()

print ("Waiting for events")
# Wait until the end
while running:
    if (gps_serial_port != None):
        (raw_data, parsed_data) = ubr.read()
        
        (lat, lon) = extract_lat_lon(str(parsed_data))
        if (lat != None):
            geohash = pygeohash.encode(lat, lon)
            if (geohash != prev_geohash):
                gpsGUI.config(text=geohash)
                set_geobot_position(trackedgeobot, geohash)
                print(lat, lon, geohash)
                prev_geohash = geohash
    else:
        gpsGUI.config(text="no gps")
            
    time.sleep(0.1)

# Close down the FSM
print ("Stopping MRServer")
MRServer.stop()

# ====================================================================================================
