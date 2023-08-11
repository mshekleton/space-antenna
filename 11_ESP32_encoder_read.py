import serial
import time

# Open the serial port
ser = serial.Serial('/dev/ttyUSB1', baudrate=115200)

# Clear the input buffer
ser.reset_input_buffer()

# Read and print the latest data from the serial port once per second
while True:
    if ser.in_waiting > 0:
        # Discard previous data
        ser.read(ser.in_waiting)

        # Read and print the latest data
        data = ser.readline().decode().strip()
        print(data)

    time.sleep(.1)
