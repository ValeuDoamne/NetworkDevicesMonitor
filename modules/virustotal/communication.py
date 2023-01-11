import socket
import struct

def client(pid_file):
    connect_socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM, 0)
    connect_socket.connect(pid_file)
    return connect_socket

def send_data(client, data):
    length = len(data)
    length_packed = struct.pack("Q", length)
    client.send(length_packed)
    data_as_bytes = bytes(data.encode('latin-1'))
    client.send(data_as_bytes)

def receive_data(client):
    length = struct.unpack("Q", conn.recv(8))
    data   = client.recv(length)
    return data.encode('latin-1')
