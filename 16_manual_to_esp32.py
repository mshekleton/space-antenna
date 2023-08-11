import serial
import socket
import time

ROTCTLD_HOST = "localhost"
ROTCTLD_PORT = 4533  # default port for rotctld
SERIAL_PORT0 = "/dev/ttyUSB0"  # adjust based on your USB port
SERIAL_PORT1 = "/dev/ttyUSB1"
SERIAL_BAUDRATE = 115200  # adjust to match ESP32 baudrate

def get_position_data():
    data = input("Enter Azimuth, Altitude: ")
    return data

def main():
    with serial.Serial(SERIAL_PORT0, SERIAL_BAUDRATE, timeout=1) as ser0:
        with serial.Serial(SERIAL_PORT1, SERIAL_BAUDRATE, timeout=1) as ser1:
            while True:
                ser1.read(ser1.in_waiting)

                # Read and print the latest data
                data = ser1.readline().decode().strip()
                print(data)
                #data = get_position_data()
                #az, el = data.split(',')
                #print(f"Azimuth: {az}, Elevation: {el}")
                #ser1.write(f"{az},{el}\n".encode())
                ser0.write(f"{data}\n".encode())
                #response = ser.readline()
                #print("Received: ", response.decode().strip())
                #time.sleep(.05)  # adjust the sleep as needed

    if ser.in_waiting > 0:
        # Discard previous data
        ser.read(ser.in_waiting)

        # Read and print the latest data
        data = ser.readline().decode().strip()
        print(data)


if __name__ == "__main__":
    main()
