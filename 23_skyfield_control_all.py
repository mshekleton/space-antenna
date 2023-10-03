import requests
import time
import serial
from skyfield.api import Topos, load, EarthSatellite
import threading
#from pynput import keyboard
import curses

SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_BAUDRATE = 115200

# List of Celestrak Group URLs
GROUP_URLS = [
    "https://www.celestrak.com/NORAD/elements/stations.txt",
    "https://www.celestrak.com/NORAD/elements/noaa.txt",
    "https://www.celestrak.com/NORAD/elements/gps-ops.txt",
    "https://www.celestrak.com/NORAD/elements/goes.txt",
    "https://www.celestrak.com/NORAD/elements/resource.txt",
    "https://www.celestrak.com/NORAD/elements/weather.txt",
    "https://www.celestrak.com/NORAD/elements/sarsat.txt",
    "https://www.celestrak.com/NORAD/elements/engineering.txt",
    "https://www.celestrak.com/NORAD/elements/education.txt",
    "https://www.celestrak.com/NORAD/elements/cubesat.txt",
    "https://www.celestrak.com/NORAD/elements/geodetic.txt",
    "https://www.celestrak.com/NORAD/elements/intelsat.txt",
    "https://www.celestrak.com/NORAD/elements/orbcomm.txt",
    "https://www.celestrak.com/NORAD/elements/globalstar.txt",
    "https://www.celestrak.com/NORAD/elements/amateur.txt",
    "https://www.celestrak.com/NORAD/elements/x-comm.txt",
    "https://www.celestrak.com/NORAD/elements/cubesat.txt",
    "https://www.celestrak.com/NORAD/elements/science.txt",
    "https://www.celestrak.com/NORAD/elements/education.txt",
    "http://celestrak.org/NORAD/elements/gp.php?GROUP=last-30-days&FORMAT=tle",
]

def get_tle_data(url):
    response = requests.get(url)
    response.raise_for_status()
    
    lines = response.text.strip().split("\n")
    satellites = {}
    
    for idx in range(0, len(lines), 3):
        satellite_name = lines[idx].strip()
        satellites[satellite_name] = (lines[idx], lines[idx + 1], lines[idx + 2])
    
    return satellites

def get_satellite_tle(stdscr):
    #stdscr.clear()
    #curses.cbreak()
    curses.halfdelay(1)  # waits for 0.1 seconds for user input
    curses.echo()
    curses.curs_set(1)  # Show cursor
    stdscr.nodelay(0)  # Set to blocking mode

    def get_user_choice(prompt, items):
        current_top = 0  # Index of the top item being displayed
        page_size = curses.LINES - 3  # Number of items to display at once (leave room for prompt and errors)
        while True:
            stdscr.clear()
            # Ensure the window size is enough
            if page_size < 1:
                stdscr.addstr("Please increase the size of the terminal window.\n")
                stdscr.refresh()
                time.sleep(2)  # Wait for 2 seconds before checking again
                page_size = curses.LINES - 3  # Update page size
                continue
            # Display items
            for idx, item in enumerate(items[current_top:current_top + page_size], start=current_top + 1):
                stdscr.addstr(f"{idx}. {item}\n")
            stdscr.addstr(prompt)
            stdscr.refresh()
            stdscr.nodelay(0)  # Set to blocking mode
            stdscr.timeout(-1)  # Wait indefinitely for input
            try:
                choice = int(stdscr.getstr().decode('utf-8'))
                if 1 <= choice <= len(items):
                    return choice
                else:
                    stdscr.addstr("Invalid input. Please enter a number within the valid range.\n")
                    stdscr.refresh()
            except ValueError:
                stdscr.addstr("Invalid input. Please enter a number.\n")
                stdscr.refresh()
            c = stdscr.getch()  # Get character for scrolling
            if c == curses.KEY_UP and current_top > 0:
                current_top -= 1  # Scroll up
            elif c == curses.KEY_DOWN and current_top + page_size < len(items):
                current_top += 1  # Scroll down

    # Prompt the user to select a group
    group_choice = get_user_choice("Select a group by its number: ", GROUP_URLS)

    # Get TLE data from the selected group
    selected_group_url = GROUP_URLS[group_choice - 1]
    satellites = get_tle_data(selected_group_url)

    # Clear the screen
    #stdscr.clear()


    def list_satellites(satellites, current_top):
        stdscr.clear()
        page_size = curses.LINES - 3
        for idx, name in enumerate(list(satellites.keys())[current_top:current_top + page_size], start=current_top + 1):
            stdscr.addstr(f"{idx}. {name}\n")
        stdscr.refresh()

    current_top = 0
    page_size = curses.LINES - 3
    while True:
        list_satellites(satellites, current_top)
        c = stdscr.getch()
        if c == curses.KEY_UP and current_top > 0:
            current_top -= 1
        elif c == curses.KEY_DOWN and current_top + page_size < len(satellites):
            current_top += 1
        elif c == 10:  # Enter key
            break

    satellite_choice = get_user_choice("Select a satellite by its number: ", list(satellites.keys()))

    # Return the TLE data for the selected satellite
    selected_satellite = list(satellites.keys())[satellite_choice - 1]
    return satellites[selected_satellite]

exit_flag = False  # Global flag to control loop exit

def user_input(stdscr):
    global exit_flag
    curses.curs_set(0)  # Hide cursor
    stdscr.nodelay(1)  # Set to non-blocking mode
    while not exit_flag:
        c = stdscr.getch()
        if c == ord('x'):
            exit_flag = True
            break


def main(stdscr):
    # Initialize the serial port
    ser = serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=1)
    ser.flush()

    global exit_flag  # Make sure to use the global exit_flag variable

    try:
        while not exit_flag:  # Check exit_flag in your loop condition
            # Start user input thread
            input_thread = threading.Thread(target=user_input, args=(stdscr,))
            input_thread.daemon = True  # Daemonize thread
            input_thread.start()

            # Get satellite TLE data
            line1, line2, line3 = get_satellite_tle(stdscr)
            
            # Load TLE data into Skyfield EarthSatellite object
            satellite = EarthSatellite(line2, line3, line1)
            
            # Load Skyfield data
            ts = load.timescale()
            
            # Get user location
            lat = 41.81472227118011
            lon = -72.48343714114537
            observer = Topos(latitude_degrees=lat, longitude_degrees=lon)

            # Tracking loop
            while not exit_flag:
                t0 = ts.now()
                difference = satellite - observer
                topocentric = difference.at(t0)
                
                alt, az, d = topocentric.altaz()
                
                # Send data to serial port
                if alt.degrees > 0:
                    data_string = f"{az.degrees:.2f}, {alt.degrees:.2f}\n"
                else:
                    data_string = "0.00, 0.00\n"
                #print(data_string)
                y, x = stdscr.getyx()  # Get current position of the cursor
                stdscr.move(y-1, 0)  # Move cursor to the beginning of the display line
                stdscr.clrtoeol()  # Clear to the end of the line to remove old text
                stdscr.addstr(y-1, 0, "Azimuth, Altitude: " + data_string)  # Display the new text
                stdscr.refresh()  # Update the screen

                ser.write(data_string.encode('utf-8'))
                
                time.sleep(.1)

            input_thread.join() # Wait for input_thread to exit

            # Reset exit_flag for next iteration
            exit_flag = False

    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        ser.close()
        curses.nocbreak()  # Switch back to canonical input mode
        curses.endwin() # Restore the terminal to its original state

if __name__ == "__main__":
    curses.wrapper(main)
