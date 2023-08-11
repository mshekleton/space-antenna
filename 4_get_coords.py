import socket

HOST = '127.0.0.1' # Standard loopback address
#PORT = 65432
PORT = 5353

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect(('', PORT))
    s.sendall(b"?")
    data = s.recv(1024)

print(f"Recieved {data!r}")


    # s.bind((HOST, PORT))
    # s.listen()
    # conn, addr = s.accept()
    # with conn:
    #     print(f"connected by {addr}")
    #     while True:
    #         data = conn.recv(1024)
    #         if not data:
    #             break
    #         conn.sendall(data)