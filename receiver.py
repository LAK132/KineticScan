import socket
import struct

class MySocket:
    def __init__(self, sock = None):
        if sock is None:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        else:
            self.sock = sock

    def connect(self, host, port):
        self.sock.connect((host, port))

    def send(self, msg):
        total_sent = 0
        while total_sent < len(msg):
            sent = self.sock.send(msg[total_sent:])
            if sent == 0:
                raise RuntimeError("Socket machine broke")
            total_sent = total_sent + sent

    def read(self, length):
        chunks = []
        total_read = 0
        while total_read < length:
            chunk = self.sock.recv(min(length - total_read, 2048))
            if chunk == b'':
                raise RuntimeError("Socket machine broke")
            chunks.append(chunk)
            total_read = total_read + len(chunk)
        return b''.join(chunks)

    def send_u8(self, val):
        self.send(val.to_bytes(1, byteorder='little', signed=False))

    def read_u8(self):
        return int.from_bytes(self.read(1), byteorder='little', signed=False)

    def send_s8(self, val):
        self.send(val.to_bytes(1, byteorder='little', signed=True))

    def read_s8(self, val):
        return int.from_bytes(self.read(1), byteorder='little', signed=True)

    def send_u16(self, val):
        self.send(val.to_bytes(2, byteorder='little', signed=False))

    def read_u16(self):
        return int.from_bytes(self.read(2), byteorder='little', signed=False)

    def send_s16(self, val):
        self.send(val.to_bytes(2, byteorder='little', signed=True))

    def read_s16(self):
        return int.from_bytes(self.read(2), byteorder='little', signed=True)

    def send_u32(self, val):
        self.send(val.to_bytes(4, byteorder='little', signed=False))

    def read_u32(self):
        return int.from_bytes(self.read(4), byteorder='little', signed=False)

    def send_s32(self, val):
        self.send(val.to_bytes(4, byteorder='little', signed=True))

    def read_s32(self):
        return int.from_bytes(self.read(4), byteorder='little', signed=True)

    def send_u64(self, val):
        self.send(val.to_bytes(8, byteorder='little', signed=False))

    def read_u64(self):
        return int.from_bytes(self.read(8), byteorder='little', signed=False)

    def send_s64(self, val):
        self.send(val.to_bytes(8, byteorder='little', signed=True))

    def read_s64(self):
        return int.from_bytes(self.read(8), byteorder='little', signed=True)

# create an INET, STREAMing socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
# bind the socket to a public host, and a well-known port
server_socket.bind(('127.0.0.1', 13269))

try:
    # become a server socket
    server_socket.listen(5)

    while True:
        (clientsocket, address) = server_socket.accept()
        # accept connections from outside
        # now do something with the clientsocket
        my_socket = MySocket(clientsocket)
        # my_socket.send_u32(132)

        try:
            while True:
                width = my_socket.read_u32()
                height = my_socket.read_u32()

                my_socket.read(width * height * 4)

                print('Width ', width)
                print('Height ', height)

                width = my_socket.read_u32()
                height = my_socket.read_u32()

                my_socket.read(width * height * 2)

                print('Width ', width)
                print('Height ', height)
        except Exception:
            print("Exception while reading")

        clientsocket.shutdown(socket.SHUT_RDWR)
        clientsocket.close()
except Exception:
    pass

print("Shutting down")
server_socket.shutdown(socket.SHUT_RDWR)
server_socket.close()
