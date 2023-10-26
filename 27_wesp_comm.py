import requests

# URL of your ESP32 server
url = "http://10.42.0.39/echo"

# Data to be sent
#data = "az: 123.45, alt: 67.89"
data = input("Enter data to send: ")

# Headers to be sent
headers = {"Content-Type": "text/plain"}

# Sending the POST request and storing the response
while True:
    try:
        response = requests.post(url, data=data, headers=headers)

        # Output the server's response
        print("Status Code: ", response.status_code)
        print("Response Text: ", response.text)
        
    except requests.exceptions.RequestException as e:
        # Output any network-related errors
        print("Error:", e)
