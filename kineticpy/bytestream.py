class ByteStream:
    def __init__(self, byte_stream):
        self.stream = byte_stream

    def write(self, buffer):
        self.stream.write(buffer)

    def read(self, length):
        return self.stream.read(length)

    def write_u8(self, val):
        self.write(val.to_bytes(1, byteorder='little', signed=False))

    def read_u8(self):
        return int.from_bytes(self.read(1), byteorder='little', signed=False)

    def write_s8(self, val):
        self.write(val.to_bytes(1, byteorder='little', signed=True))

    def read_s8(self, val):
        return int.from_bytes(self.read(1), byteorder='little', signed=True)

    def write_u16(self, val):
        self.write(val.to_bytes(2, byteorder='little', signed=False))

    def read_u16(self):
        return int.from_bytes(self.read(2), byteorder='little', signed=False)

    def write_s16(self, val):
        self.write(val.to_bytes(2, byteorder='little', signed=True))

    def read_s16(self):
        return int.from_bytes(self.read(2), byteorder='little', signed=True)

    def write_u32(self, val):
        self.write(val.to_bytes(4, byteorder='little', signed=False))

    def read_u32(self):
        return int.from_bytes(self.read(4), byteorder='little', signed=False)

    def write_s32(self, val):
        self.write(val.to_bytes(4, byteorder='little', signed=True))

    def read_s32(self):
        return int.from_bytes(self.read(4), byteorder='little', signed=True)

    def write_u64(self, val):
        self.write(val.to_bytes(8, byteorder='little', signed=False))

    def read_u64(self):
        return int.from_bytes(self.read(8), byteorder='little', signed=False)

    def write_s64(self, val):
        self.write(val.to_bytes(8, byteorder='little', signed=True))

    def read_s64(self):
        return int.from_bytes(self.read(8), byteorder='little', signed=True)
