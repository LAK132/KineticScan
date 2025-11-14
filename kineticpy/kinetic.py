import socket
from contextlib import contextmanager

from bytestream import ByteStream
from socketstream import SocketStream

class KineticStream():
    def __init__(self, client):
        self.client = client

    def frame(self):
        colour_width = self.stream.read_u32()
        colour_height = self.stream.read_u32()
        colour_frame = self.stream.read(colour_width * colour_height * 4)
        depth_width = self.stream.read_u32()
        depth_height = self.stream.read_u32()
        depth_frame = self.stream.read(depth_width * depth_height * 2)
        return colour_width, colour_height, colour_frame, depth_width, depth_height, depth_frame

    def frames(self):
        while True:
            yield self.frame()


class KineticSocket():
    def __init__(self, socket):
        self.socket = socket

    @contextmanager
    def accept(self):
        (client_socket, address) = self.socket.accept()
        try:
            yield KineticStream(client_socket)
        except Exception as e:
            try:
                client_socket.shutdown(socket.SHUT_RDWR)
            except:
                pass
            client_socket.close()
            raise e
        try:
            client_socket.shutdown(socket.SHUT_RDWR)
        except:
            pass
        client_socket.close()



@contextmanager
def kinectic_socket(address='127.0.0.1', port=13279):
    socket = socket.socket(socket.AF_INET,
                           socket.SOCK_STREAM,
                           socket.IPPROTO_TCP)
    try:
        socket.bind((address, port))
        socket.listen(5)
        yield

class KineticStream():
    def __enter__(self, socket):
        self.stream = ByteStream(SocketStream(socket))

    def __exit__(self):

    def read_frame(self):
        colour_width = self.stream.read_u32()
        colour_height = self.stream.read_u32()
        colour_frame = self.stream.read(colour_width * colour_height * 4)
        depth_width = self.stream.read_u32()
        depth_height = self.stream.read_u32()
        depth_frame = self.stream.read(depth_width * depth_height * 2)
        return colour_width, colour_height, colour_frame, depth_width, depth_height, depth_frame

class KineticSocket():
    def __init__(self, address, port = 13279):
        self.socket = socket.socket(socket.AF_INET,
                                    socket.SOCK_STREAM,
                                    socket.IPPROTO_TCP)
        self.socket.bind((address, port))
        self.socket.listen(5)

    def accept():
        (client, address) = self.socket.accept()
        return KineticStream(client)
