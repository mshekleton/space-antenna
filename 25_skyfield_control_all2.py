from prompt_toolkit import PromptSession
from prompt_toolkit.patch_stdout import patch_stdout
import threading
from skyfield.api import Topos, load
from skyfield.timelib import Time
from skyfield.sgp4lib import EarthSatellite
import time

# Assume you have the following TLE data:
line1 = 'ISS (ZARYA)'
line2 = '1 25544U 98067A   23276.84909904  .00020699  00000+0  37284-3 0  9995'
line3 = '2 25544  51.6407 150.3150 0005472  70.0141  31.4145 15.49749870418646'

def tracking_loop():
    # Load TLE data into Skyfield EarthSatellite object
    satellite = EarthSatellite(line2, line3, line1)
    ts = load.timescale()
    lat, lon = 41.81472227118011, -72.48343714114537
    observer = Topos(latitude_degrees=lat, longitude_degrees=lon)

    while True:  # You might want to have a condition to exit this loop
        t0 = ts.now()
        difference = satellite - observer
        topocentric = difference.at(t0)
        alt, az, d = topocentric.altaz()
        data_string = f"Azimuth: {az.degrees:.2f}, Altitude: {alt.degrees:.2f}"
        print(data_string, end='\r', flush=True)  # Update the display on the same line
        time.sleep(1)  # Update every second

def main():
    with patch_stdout():
        tracking_thread = threading.Thread(target=tracking_loop)
        tracking_thread.daemon = True  # Daemonize thread
        tracking_thread.start()

        session = PromptSession()
        user_input = ''
        while user_input.lower() != 'q':
            user_input = session.prompt("Enter 'q' to quit: ")

if __name__ == "__main__":
    main()
