import socket
import serial
import time
import threading

SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_BAUDRATE = 115200

class MockRotator:
    def __init__(self):
        self.azimuth = 0.0
        self.elevation = 0.0
        # Start the serial communication in a separate thread
        self.serial_thread = threading.Thread(target=self.start_serial_comm)
        self.serial_thread.daemon = True  # This will let the thread exit when the main program exits
        self.serial_thread.start()

    def start_serial_comm(self):
        with serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=1) as ser:
            while True:
                self.write_to_serial(ser)
                #time.sleep(1)

    def set_position(self, az, el):
        self.azimuth = az
        self.elevation = el

    def get_position(self):
        return self.azimuth, self.elevation

    def write_to_serial(self, ser):
        data = f"{self.azimuth:.2f},{self.elevation:.2f}\n"
        print(f"Writing to serial: {data.strip()}")
        ser.write(data.encode())
        response = ser.readline()
        print("Received from serial:", response.decode().strip())


class SimpleRotctld:
    def __init__(self, host='127.0.0.1', port=4533):
        self.server_address = (host, port)
        self.rotator = MockRotator()

    def handle_client(self, conn, addr):
        print("Connected by", addr)
        while True:
            data = conn.recv(1024)
            if not data:
                break

            cmd = data.decode().strip().split()
            if cmd[0] == "P":
                az, el = float(cmd[1]), float(cmd[2])
                self.rotator.set_position(az, el)
                response = "RPRT 0\n"
            elif cmd[0] == "p":
                az, el = self.rotator.get_position()
                response = f"{az:.2f}\n{el:.2f}\n"
            else:
                response = "RPRT -1\n"

            conn.send(response.encode())
        print("Client", addr, "disconnected")

    def run(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind(self.server_address)
            s.listen()

            while True:
                print("Waiting for connection...")
                conn, addr = s.accept()
                with conn:
                    self.handle_client(conn, addr)

if __name__ == '__main__':
    server = SimpleRotctld()
    server.run()
