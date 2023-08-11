import socket

def start_daemon(port=4533):
    # Create a server socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Allow reuse of the address
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    # Bind the socket to the port
    server_socket.bind(('0.0.0.0', port))
    
    # Make the server listen for incoming connections
    server_socket.listen(5)
    print(f"Daemon listening on port {port}")
    
    while True:
        # Accept a connection
        client_socket, client_address = server_socket.accept()
        
        # Receive data from the client
        data = client_socket.recv(1024).decode().strip()
        
        # Print the received data
        print(f"Received from {client_address}: {data}")
        
        # Check if the received data is 'p' and respond accordingly
        if data == 'p':
            client_socket.sendall(b'0 0')
        
        # Close the client socket
        client_socket.close()

if __name__ == "__main__":
    start_daemon(4533)
