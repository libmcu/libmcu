import sys
import fcntl
import os
import struct

TYPE_LIST = ("VERBOSE", "DEBUG", "INFO", "WARN", "ERROR", "NONE")
TIMESTAMP_SIZE = 4 # 8 or 4 bytes
LOG_SIZE = TIMESTAMP_SIZE + 4*2 + 2 + 2 + 1 # sizeof(ts + pc + lr + len + type)
LOG_MAGIC = 0xA5A5


class embedlog:
    def __init__(self):
        self.elf_file = None

    def __str__(self):
        self.message = self.message.replace('\n', ' ').replace('\r', ' ')
        log_string = "{ts:012d}:".format(ts=self.timestamp)

        if self.elf_file is None:
            log_string += " <pc@{pc:#x},lr@{lr:#x}>".format(pc=self.pc, lr=self.lr)

        log_string += " [{type}] {msg}".format(type=conv_type_to_string(self.log_type), msg=self.message)

        if self.elf_file is not None:
            log_string += "\n            pc@" \
                + addr2line(self.elf_file, self.pc)
            log_string += "\n            lr@" \
                + addr2line(self.elf_file, self.lr)

        return log_string

    def check_if_valid(self):
        """Check if log is valid

        Args:
            pc (int): Program Counter
            lr (int): Return Address
            magic (int): Checksum
        Returns:
            bool
        """
        return ((self.pc ^ self.lr ^ self.magic) & 0xffff) == LOG_MAGIC

    def unpack_log(self, byte_stream):
        """Unpack a byte stream into a log

        Args:
            byte_stream (bytes): A byte stream
        Returns:
            int: The number of bytes successfully read from `byte_stream`
        Raises:
            struct.error: Raised when reaching at the end of the stream
        """
        base_idx = TIMESTAMP_SIZE
        if base_idx == 8:
            ts_fmt = "<Q"
        else:
            ts_fmt = "<L"
        self.timestamp = struct.unpack(ts_fmt, byte_stream[0:base_idx])[0]
        self.pc = struct.unpack("<L", byte_stream[base_idx:base_idx+4])[0]
        self.lr = struct.unpack("<L", byte_stream[base_idx+4:base_idx+8])[0]
        self.magic = struct.unpack("<H", byte_stream[base_idx+8:base_idx+10])[0]

        if self.check_if_valid() is not True:
            return 1

        self.message_length = struct.unpack("H", byte_stream[base_idx+10:base_idx+12])[0]
        self.log_type = struct.unpack("B", byte_stream[base_idx+12:base_idx+13])[0]
        self.message = struct.unpack(str(self.message_length) + "s",
                                     byte_stream[base_idx+13:base_idx+13+self.message_length])[0].decode('ascii')

        return self.message_length + LOG_SIZE


def addr2line(file, addr):
    cmd = os.getenv("ADDR2LINE", "addr2line")
    cmd += " -pfiC -a 0x{0:x} -e {1:s}".format(addr, file)
    return os.popen(cmd).read().split('\n')[0]

def conv_type_to_string(log_type):
    return TYPE_LIST[log_type]


if __name__ == "__main__":
    # make stdin a non-blocking file
    fd = sys.stdin.fileno()
    fl = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)

    log = embedlog()

    if len(sys.argv) == 2 and os.path.isfile(sys.argv[1]):
        log.elf_file = sys.argv[1]

    while (True):
        stream = sys.stdin.buffer.read()
        if stream is not None and len(stream) > 0:
            while True:
                try:
                    bytes_read = log.unpack_log(stream)
                    stream = stream[bytes_read:]
                    if bytes_read >= LOG_SIZE:
                        print(log)
                except struct.error:
                    break
