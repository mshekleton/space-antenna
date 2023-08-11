import serial
import socket
import time
import subprocess
import os
import signal


ROTCTLD_HOST = "localhost"
ROTCTLD_PORT = 4533  # default port for rotctld
SERIAL_PORT = "/dev/ttyUSB0"  # adjust based on your USB port
SERIAL_BAUDRATE = 115200  # adjust to match ESP32 baudrate



def is_process_running(process_name):
    """Check if there is any running process that contains the given name process_name."""
    call = 'pgrep -f ' + process_name
    output = subprocess.check_output(call, shell=True)
    if output.strip():
        return True
    else:
        return False

def get_pid(process_name):
    """Return pid of the process with the given name."""
    try:
        return int(subprocess.check_output(["pidof", "-s", process_name]))
    except:
        return None

def start_process(process_name):
    """Start the process with the given name."""
    subprocess.Popen(process_name)

# # Check if rotctld is already running
# if is_process_running("rotctld"):
#     print("rotctld is already running")
#     # If it's running then restart it
#     pid = get_pid("rotctld")
#     if pid:
#         # Send SIGTERM signal to the process
#         os.kill(pid, signal.SIGTERM)
#         time.sleep(1)  # Give it time to terminate
#         if not is_process_running("rotctld"):  # Ensure it terminated
#             print("rotctld has been terminated, restarting it now.")
#             start_process("rotctld")
#             print("rotctld has been restarted.")
#         else:
#             print("Failed to terminate rotctld.")
# else:
#     print("rotctld is not running, starting it now.")
#     start_process("rotctld")
#     print("rotctld has been started.")


# # Launch gpredict
#start_process("gpredict")
#start_process("rotctld")
# print("gpredict has been launched.")
#time.sleep(5)

# def get_rotctld_data():
#     with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#         s.connect((ROTCTLD_HOST, ROTCTLD_PORT))
#         s.sendall(b'p\n')  # command to get azimuth and elevation
#         data = s.recv(1024)
#         return data.decode('utf-8').strip()

def get_rotctld_status():
    try:
        # Using subprocess to get the service status
        result = subprocess.check_output(['systemctl', 'status', 'rotctld'], stderr=subprocess.STDOUT).decode('utf-8')
        return result
    except subprocess.CalledProcessError as e:
        return e.output.decode('utf-8')

def get_rotctld_data():
    while True:
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((ROTCTLD_HOST, ROTCTLD_PORT))
                s.sendall(b'p\n')
                data = s.recv(1024)
                return data.decode('utf-8').strip()
        except ConnectionRefusedError:
            print("Connection refused. Checking rotctld status...")
            print(get_rotctld_status())
            print("Retrying in 2 seconds...")
            time.sleep(2)  # sleep for 2 seconds before retrying




def main():
    with serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=1) as ser:
        while True:
            data = get_rotctld_data()
            az, el = data.split('\n')
            print(f"Azimuth: {az}, Elevation: {el}")
            ser.write(f"{az},{el}\n".encode())
            response = ser.readline()
            print("Received: ", response.decode().strip())
            time.sleep(1)  # adjust the sleep as needed

if __name__ == "__main__":
    main()
