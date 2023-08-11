import time
import curses
from tabulate import tabulate
import socket
import serial
import subprocess


DEBUG = False

# Initialize variables
altitude_request = 10
azimuth_request = 20
altitude_position = 10
azimuth_position = 20
altitude_quad = 15
azimuth_quad = 25
altitude_quad_mode = "+"
azimuth_quad_mode = "OFF"
import RPi.GPIO as GPIO


GPIO.setmode(GPIO.BCM) 
GPIO.setwarnings(False)

#set up the pins we have been using
#sw = 27
direction = 20
step = 21
 
#set up the GPIO events on those pins
GPIO.setup(step, GPIO.OUT)
GPIO.setup(direction, GPIO.OUT)

# Define rotctld command
#rotctld_path = r'C:\Program Files\hamlib-w64-4.5.5\bin\rotctld.exe'
#rotctld_command = [rotctld_path]

# Start rotctld daemon as a subprocess
#daemon_process = subprocess.Popen(rotctld_command)
daemon_process = subprocess.Popen('rotctld')
time.sleep(2)

# Open the serial port
ser = serial.Serial('/dev/ttyUSB0', baudrate=115200)

# Clear the input buffer
ser.reset_input_buffer()

def get_rotor_position(host="localhost", port=4533):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((host, port))
        s.sendall(b'p\x0a')  # 'p' command is for getting position
        data = s.recv(1024)
        decoded_data = data.decode('utf-8')
        alt_az_values = decoded_data.split()
        if len(alt_az_values) == 2:
            azimuth = float(alt_az_values[0])
            altitude = float(alt_az_values[1])
            if DEBUG is True: print(f"Azimuth: {azimuth}")
            if DEBUG is True: print(f"Altitude: {altitude}")
        #time.sleep(CYCLE)  # adjust delay as needed
    return(azimuth, altitude)

def get_manual_position():
    global azimuth_quad
    if ser.in_waiting > 0:
        # Discard previous data
        ser.read(ser.in_waiting)

        # Read and print the latest data
        data = ser.readline().decode().strip()
        azimuth_quad = data
        if DEBUG is True: print(data)
        return float(data)
    return float(0.0)



def print_table(stdscr):
    curses.curs_set(0)
    while True:
        # Change variables in whatever way you need here
        global altitude_request
        global azimuth_request
        global azimuth_position
        global altitude_position
        azimuth_request, altitude_request = get_rotor_position()
        azimuth_quad = get_manual_position()
        #altitude_quad = get_manual_position()
        altitude_quad = 0
        
        # Generate table
        table = [
            [
                "Altitude",
                altitude_position,
                altitude_request,
                altitude_position - altitude_request,
                altitude_quad,
                altitude_quad_mode,
            ],
            [
                "Azimuth",
                azimuth_position,
                azimuth_request,
                azimuth_position - azimuth_request,
                azimuth_quad,
                azimuth_quad_mode,
            ],
        ]
        formatted_table = tabulate(
            table,
            headers=["", "Position", "Request", "Delta", "Manual", "Mode"],
            tablefmt="fancy_grid",
        )

        # Clear screen and print table
        stdscr.clear()
        stdscr.addstr(0, 0, formatted_table)
        stdscr.refresh()

        #time.sleep(.01)  # Wait for 1 second before next update

        # TEMPORARY
        azimuth_request = azimuth_quad

        sleep_period = 0.005
        if azimuth_position > azimuth_request:
            GPIO.output(direction, 1)
            GPIO.output(step, 1)
            time.sleep(sleep_period)
            GPIO.output(step, 0)
            time.sleep(sleep_period)
            azimuth_position = azimuth_position - 1
        elif azimuth_position < azimuth_request:
            GPIO.output(direction, 0)
            GPIO.output(step, 1)
            time.sleep(sleep_period)
            GPIO.output(step, 0)
            time.sleep(sleep_period)
            azimuth_position = azimuth_position + 1
        else: #azimuth_position == azimuth_request:
            time.sleep(2*sleep_period)


# Run the function using curses wrapper
#curses.wrapper(print_table)
sleep_period = 0.001
while True:
    #get_manual_position()
    GPIO.output(direction, 1)
    GPIO.output(step, 1)
    time.sleep(sleep_period)
    GPIO.output(step, 0)
    time.sleep(sleep_period)


# Terminate the rotctld daemon process when you're finished
daemon_process.terminate()