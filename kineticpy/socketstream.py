class SocketStream:
    def __init__(self, sock):
        self.sock = sock

    def write(self, msg):
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
