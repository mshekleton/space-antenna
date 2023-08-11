import socket

class MockRotator:
    def __init__(self):
        self.azimuth = 0.0
        self.elevation = 0.0

    def set_position(self, az, el):
        self.azimuth = az
        self.elevation = el
        print("Position set: " + str(az) + ", " + str(el))

    def get_position(self):
        return self.azimuth, self.elevation


class SimpleRotctld:
    def __init__(self, host='127.0.0.1', port=4533):
        self.server_address = (host, port)
        self.rotator = MockRotator()

    def handle_client(self, conn, addr):
        print("Connected by", addr)
        while True:
            data = conn.recv(1024)
            if not data:
                break

            cmd = data.decode().strip().split()
            if cmd[0] == "P":
                az, el = float(cmd[1]), float(cmd[2])
                self.rotator.set_position(az, el)
                response = "RPRT 0\n"
            elif cmd[0] == "p":
                az, el = self.rotator.get_position()
                response = f"{az:.1f}\n{el:.1f}\n"
            else:
                response = "RPRT -1\n"

            conn.send(response.encode())
        print("Client", addr, "disconnected")

    def run(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind(self.server_address)
            s.listen()

            while True:
                print("Waiting for connection...")
                conn, addr = s.accept()
                with conn:
                    self.handle_client(conn, addr)

if __name__ == '__main__':
    server = SimpleRotctld()
    server.run()
