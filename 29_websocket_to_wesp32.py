# import websocket
# import time
# import threading


# def on_message(ws, message):
#     print(f"Received: {message}")

# def on_error(ws, error):
#     print(f"Error: {error}")

# def on_close(ws, close_status_code, close_msg):
#     print("Socket Closed")

# def on_open(ws):
#     def run(*args):
#         #print("Sending message: Hello, Server")
#         #ws.send("Hello, Server")
#         ws.send(input('Send message: '))
#         #time.sleep(1)
#         #ws.close()
#     threading.Thread(target=run).start()

# # Update the URL to your ESP32 WebSocket server's IP and port
# socket_url = "ws://10.42.0.39:81"

# # Create a WebSocket connection
# ws = websocket.WebSocketApp(socket_url,
#                             on_message=on_message,
#                             on_error=on_error,
#                             on_close=on_close)

# ws.on_open = on_open

# # Start the WebSocket connection
# ws.run_forever()


# import serial
# import websocket
# import threading
# import time

# SERIAL_PORT1 = "/dev/ttyUSB1"
# SERIAL_BAUDRATE = 115200

# WEBSOCKET_URL = "ws://10.42.0.39:81"  # Adjust to your ESP32's IP and port

# def on_message(ws, message):
#     print(f"Received: {message}")

# def on_error(ws, error):
#     print(f"Error: {error}")

# def on_close(ws, close_status_code, close_msg):
#     print("Socket Closed")

# def on_open(ws):
#     def run(*args):
#         with serial.Serial(SERIAL_PORT1, SERIAL_BAUDRATE, timeout=1) as ser1:
#             while True:
#                 # Make sure to read what's already in the input buffer
#                 ser1.read(ser1.in_waiting)
#                 data = ser1.readline().decode().strip()
#                 #print(f"Read from serial: {data}")
                
#                 # Sending data through WebSocket
#                 ws.send(data)
#                 #time.sleep(1)  # Adjust as per your requirement

#     threading.Thread(target=run).start()

# def main():
#     ws = websocket.WebSocketApp(
#         WEBSOCKET_URL,
#         on_message=on_message,
#         on_error=on_error,
#         on_close=on_close
#     )

#     ws.on_open = on_open
#     ws.run_forever()

# if __name__ == "__main__":
#     main()


# import serial
# import websocket
# import threading
# import time

# SERIAL_PORT1 = "/dev/ttyUSB1"
# SERIAL_BAUDRATE = 115200
# WEBSOCKET_URL = "ws://10.42.0.39:81"

# def on_message(ws, message):
#     print(f"Received: {message}")

# def on_error(ws, error):
#     print(f"Error: {error}")

# def on_close(ws, close_status_code, close_msg):
#     print("Socket Closed")

# def on_open(ws):
#     def run(*args):
#         with serial.Serial(SERIAL_PORT1, SERIAL_BAUDRATE, timeout=1) as ser1:
#             while True:
#                 ser1.read(ser1.in_waiting)
#                 data = ser1.readline().decode().strip()
#                 print(f"Read from serial: {data}")
                
#                 # Sending data through WebSocket
#                 ws.send(data)
#                 #time.sleep(1)  # Adjust as per your requirement

#     threading.Thread(target=run).start()

# def main():
#     while True:  # Reconnect loop
#         try:
#             ws = websocket.WebSocketApp(
#                 WEBSOCKET_URL,
#                 on_message=on_message,
#                 on_error=on_error,
#                 on_close=on_close
#             )
#             ws.on_open = on_open
#             ws.run_forever()

#         except websocket.WebSocketException as e:
#             print(f"WebSocketException: {e}")
#             print("Retrying in 5 seconds...")
#             time.sleep(5)

#         except Exception as e:
#             print(f"Exception: {e}")
#             print("Retrying in 5 seconds...")
#             time.sleep(5)

# if __name__ == "__main__":
#     main()


import serial
import websocket
import threading
import time

SERIAL_PORT1 = "/dev/ttyUSB1"
SERIAL_BAUDRATE = 115200
WEBSOCKET_URL = "ws://10.42.0.39:81"

# def on_message(ws, message):
#     print(f"Received: {message}")

def on_error(ws, error):
    print(f"Error: {error}")

def on_close(ws, close_status_code, close_msg):
    print("Socket Closed")

def on_open(ws):
    def run(*args):
        with serial.Serial(SERIAL_PORT1, SERIAL_BAUDRATE, timeout=1) as ser1:
            last_heartbeat_reply = time.time()
            while True:
                ser1.read(ser1.in_waiting)
                data = ser1.readline().decode().strip()
                print(f"Read from serial: {data}")

                # Sending data through WebSocket
                ws.send(data)
                time.sleep(1)

                # Send a heartbeat message
                ws.send("HEARTBEAT")
                
                # Check for the last heartbeat reply
                if time.time() - last_heartbeat_reply > 5:  # 5 seconds timeout
                    print("Heartbeat lost! Closing connection.")
                    ws.close()
                    break

    thread = threading.Thread(target=run)
    thread.daemon = True
    thread.start()

def on_message(ws, message):
    print(f"Received: {message}")
    if message == "HEARTBEAT_ACK":  # expected reply to heartbeat
        last_heartbeat_reply = time.time()


def main():
    ws = None
    try:
        while True:  # Reconnect loop
            try:
                ws = websocket.WebSocketApp(
                    WEBSOCKET_URL,
                    on_message=on_message,
                    on_error=on_error,
                    on_close=on_close
                )
                ws.on_open = on_open
                ws.run_forever(ping_interval=10, ping_timeout=5)  # Adjust timing as needed

            except websocket.WebSocketException as e:
                print(f"WebSocketException: {e}")
                print("Retrying in 5 seconds...")
                time.sleep(5)

            except Exception as e:
                print(f"Exception: {e}")
                print("Retrying in 5 seconds...")
                time.sleep(5)
    
    except KeyboardInterrupt:
        print("Attempting to close connections...")
        if ws:
            ws.close()
        print("Connections closed. Exiting.")


if __name__ == "__main__":
    main()
