import requests
import time
import serial
from skyfield.api import Topos, load, EarthSatellite
import threading
#from pynput import keyboard
from rich.live import Live
import multiprocessing

SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_BAUDRATE = 115200


exit_flag = False

# def user_input():
#     global exit_flag
#     while not exit_flag:
#         user_command = input("Enter 'q' to quit: ")
#         if user_command.lower() == 'q':
#             exit_flag = True

# def user_input():
#     global exit_flag
#     user_input = input("Enter 'q' to quit: ")
#     if user_input.lower() == 'q':
#         exit_flag = True

def user_input(exit_flag):
    user_input = input("Enter 'q' to quit: ")
    if user_input.lower() == 'q':
        exit_flag.value = True

GROUP_URLS = {
    "Stations": "https://www.celestrak.com/NORAD/elements/stations.txt",
    "NOAA": "https://www.celestrak.com/NORAD/elements/noaa.txt",
    "GPS Operations": "https://www.celestrak.com/NORAD/elements/gps-ops.txt",
    "Geostationary Orbit": "https://www.celestrak.com/NORAD/elements/goes.txt",
    "Resource Satellites": "https://www.celestrak.com/NORAD/elements/resource.txt",
    "Weather": "https://www.celestrak.com/NORAD/elements/weather.txt",
    "Search and Rescue": "https://www.celestrak.com/NORAD/elements/sarsat.txt",
    "Engineering": "https://www.celestrak.com/NORAD/elements/engineering.txt",
    "Education": "https://www.celestrak.com/NORAD/elements/education.txt",
    "CubeSats": "https://www.celestrak.com/NORAD/elements/cubesat.txt",
    "Geodetic": "https://www.celestrak.com/NORAD/elements/geodetic.txt",
    "Intelsat": "https://www.celestrak.com/NORAD/elements/intelsat.txt",
    "Orbcomm": "https://www.celestrak.com/NORAD/elements/orbcomm.txt",
    "Globalstar": "https://www.celestrak.com/NORAD/elements/globalstar.txt",
    "Amateur Satellites": "https://www.celestrak.com/NORAD/elements/amateur.txt",
    "X-Comm": "https://www.celestrak.com/NORAD/elements/x-comm.txt",
    "Science": "https://www.celestrak.com/NORAD/elements/science.txt",
    "Last 30 Days Launches": "http://celestrak.org/NORAD/elements/gp.php?GROUP=last-30-days&FORMAT=tle",
}


def get_tle_data():
    for idx, category in enumerate(GROUP_URLS.keys(), 1):
        print(f"{idx}. {category}")

    category_idx = int(input("Select a category by number: ")) - 1
    category_name = list(GROUP_URLS.keys())[category_idx]
    category_url = GROUP_URLS[category_name]

    # Fetch satellite list
    response = requests.get(category_url)
    response.raise_for_status()  # Check for a valid response

    lines = response.text.strip().split('\n')
    satellites = [(lines[i], lines[i+1], lines[i+2]) for i in range(0, len(lines), 3)]

    for idx, (satellite_name, _, _) in enumerate(satellites, 1):
        print(f"{idx}. {satellite_name}")

    satellite_idx = int(input("Select a satellite by number: ")) - 1
    selected_satellite = satellites[satellite_idx]

    satellite_name, tle_line1, tle_line2 = selected_satellite
    print(f"Satellite: {satellite_name}")
    print(f"TLE Line 1: {tle_line1}")
    print(f"TLE Line 2: {tle_line2}")

    return satellite_name, tle_line1, tle_line2#, satellite_name

# def main(line1, line2, line3):
#     line1, line2, line3 = get_tle_data()
#     # Initialize the serial port
#     ser = serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=1)
#     ser.flush()

#     global exit_flag  # Make sure to use the global exit_flag variable

#     try:
#         # Start user input thread
#         input_thread = threading.Thread(target=user_input)
#         input_thread.daemon = True  # Daemonize thread
#         input_thread.start()

#         # Load TLE data into Skyfield EarthSatellite object
#         satellite = EarthSatellite(line2, line3, line1)
        
#         # Load Skyfield data
#         ts = load.timescale()
        
#         # Get user location
#         lat = 41.81472227118011
#         lon = -72.48343714114537
#         observer = Topos(latitude_degrees=lat, longitude_degrees=lon)

#         # Tracking loop
#         with Live(auto_refresh=False) as live:
#             while not exit_flag:
#                 t0 = ts.now()
#                 difference = satellite - observer
#                 topocentric = difference.at(t0)
                
#                 alt, az, d = topocentric.altaz()
                
#                 # Display data
#                 if alt.degrees > 0:
#                     data_string = f"{az.degrees:.2f}, {alt.degrees:.2f}\n"
#                 else:
#                     data_string = "0.00, 0.00\n"
#                 #print("Azimuth, Altitude: " + data_string)
#                 #print('\r' + data_string, end='', flush=True)  # Update display on the same line without scrolling
#                 live.update(data_string) 
            
#                 # Send data to serial port
#                 ser.write(data_string.encode('utf-8'))
                
#                 time.sleep(.1)

#         input_thread.join()  # Wait for input_thread to exit

#     except KeyboardInterrupt:
#         print("\nExiting...")
#     finally:
#         ser.close()


def main(line1, line2, line3):
    # Initialize the serial port
    SERIAL_PORT = '/dev/ttyUSB0'  # Update with your serial port
    SERIAL_BAUDRATE = 115200  # Update with your baud rate
    ser = serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=1)
    ser.flush()

    # Load TLE data into Skyfield EarthSatellite object
    satellite = EarthSatellite(line2, line3, line1)

    # Load Skyfield data
    ts = load.timescale()

    # Get user location
    lat = 41.81472227118011
    lon = -72.48343714114537
    observer = Topos(latitude_degrees=lat, longitude_degrees=lon)

    # Define the tracking loop function for threading
    def tracking_loop(exit_flag):
        with Live(auto_refresh=False) as live:
            while not exit_flag:
                t0 = ts.now()
                difference = satellite - observer
                topocentric = difference.at(t0)
                alt, az, d = topocentric.altaz()
                # Display data
                if alt.degrees > 0:
                    data_string = f"{az.degrees:.2f}, {alt.degrees:.2f}\n"
                else:
                    data_string = "0.00, 0.00\n"
                live.update(data_string)  # Update the display
                # Send data to serial port
                ser.write(data_string.encode('utf-8'))
                time.sleep(.1)

    # Use a multiprocessing.Value to share the exit_flag between processes
    exit_flag = multiprocessing.Value('b', False)

    # # Start user input thread
    # input_thread = threading.Thread(target=user_input)
    # input_thread.daemon = True  # Daemonize thread
    # input_thread.start()

    # # Start tracking loop thread
    # tracking_thread = threading.Thread(target=tracking_loop)
    # tracking_thread.daemon = True
    # tracking_thread.start()

    # # Wait for both threads to exit
    # input_thread.join()
    # tracking_thread.join()

    # Start user input process
    input_process = multiprocessing.Process(target=user_input, args=(exit_flag,))
    input_process.start()

    # Start tracking loop process
    tracking_process = multiprocessing.Process(target=tracking_loop, args=(exit_flag,))
    tracking_process.start()


    # Wait for both processes to exit
    input_process.join()
    tracking_process.join()

    # Cleanup
    ser.close()

# if __name__ == "__main__":
#     line1 = 'SATNAME'
#     line2 = 'TLE_LINE1'
#     line3 = 'TLE_LINE2'
#     main(line1, line2, line3)

if __name__ == "__main__":
    line1, line2, line3 = get_tle_data()
    main(line1, line2, line3)