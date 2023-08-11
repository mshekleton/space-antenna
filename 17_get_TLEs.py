import requests

def get_tle(satellite_name):
    url = "https://www.celestrak.com/NORAD/elements/" + satellite_name + ".txt"
    response = requests.get(url)
    if response.status_code == 200:
        tle_data = response.text.split("\n")
        tle_line1 = tle_data[0]
        tle_line2 = tle_data[1]
        return (tle_line1, tle_line2)
    else:
        return None

# Example usage:
satellite_name = "stations"  # change to the satellite name you want
tle = get_tle(satellite_name)
if tle:
    print("TLE for satellite " + satellite_name + ":")
    print(tle[0])
    print(tle[1])
else:
    print("Unable to fetch TLE data for satellite " + satellite_name)
