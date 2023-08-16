from skyfield.api import Topos, load, EarthSatellite
import requests
import time
import serial

SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_BAUDRATE = 115200

def get_iss_tle():
    url = "https://www.celestrak.com/NORAD/elements/stations.txt"
    response = requests.get(url)
    response.raise_for_status()

    lines = response.text.strip().split("\n")
    for idx, line in enumerate(lines):
        if "ISS (ZARYA)" in line:
            return lines[idx], lines[idx + 1], lines[idx + 2]
    raise ValueError("Failed to find ISS TLE data")

def main():
    # Initialize the serial port
    ser = serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=1)
    ser.flush()

    # Get the latest ISS TLE data
    line1, line2, line3 = get_iss_tle()

    # Load TLE data into Skyfield EarthSatellite object
    satellite = EarthSatellite(line2, line3, line1)

    # Load Skyfield data
    ts = load.timescale()

    # Get user location
    lat = 41.81472227118011
    lon = -72.48343714114537

    observer = Topos(latitude_degrees=lat, longitude_degrees=lon)

    try:
        while True:
            t0 = ts.now()
            difference = satellite - observer
            topocentric = difference.at(t0)

            alt, az, d = topocentric.altaz()

            print(f"\nThe ISS is at an altitude of {alt.degrees:.2f} degrees")
            print(f"The ISS is at an azimuth (elevation) of {az.degrees:.2f} degrees")

            # Send data to serial port
            data_string = f"{alt.degrees:.2f}, {az.degrees:.2f}\n"
            ser.write(data_string.encode('utf-8'))

            time.sleep(1)  # Update every 10 seconds. Change as desired.

    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        ser.close()

if __name__ == "__main__":
    main()