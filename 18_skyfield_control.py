from skyfield.api import Topos, load, EarthSatellite

def calculate_position(tle_line1, tle_line2, observer_lat, observer_lon):
    # Load satellite from provided TLE data
    satellites = load.tle(tle_line1, tle_line2)

    # Set observer's location
    observer_location = Topos(observer_lat, observer_lon)

    # Load a timescale object
    ts = load.timescale()
    t = ts.now()

    # Create an Earth object and get its position at current time
    earth = load('de421.bsp')['earth']
    earth_location = earth + observer_location

    # Calculate satellite's topocentric position
    difference = satellites - earth_location
    topocentric = difference.at(t)
    alt, az, distance = topocentric.altaz()

    return alt.degrees, az.degrees

# Example usage:
tle_line1 = '1 25544U 98067A   21274.45768913  .00001303  00000-0  26603-4 0  9992'
tle_line2 = '2 25544  51.6451 295.6731 0004417  55.4743 304.6711 15.48916314293504'
observer_lat = '37.7749 N'
observer_lon = '122.4194 W'
alt, az = calculate_position(tle_line1, tle_line2, observer_lat, observer_lon)
print(f'Altitude: {alt} degrees, Azimuth: {az} degrees')
