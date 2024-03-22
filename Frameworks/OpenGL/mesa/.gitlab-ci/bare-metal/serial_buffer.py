#!/usr/bin/env python3
#
# Copyright © 2020 Google LLC
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import argparse
from datetime import datetime, timezone
import queue
import serial
import threading
import time


class SerialBuffer:
    def __init__(self, dev, filename, prefix, timeout=None, line_queue=None):
        self.filename = filename
        self.dev = dev

        if dev:
            self.f = open(filename, "wb+")
            self.serial = serial.Serial(dev, 115200, timeout=timeout)
        else:
            self.f = open(filename, "rb")
            self.serial = None

        self.byte_queue = queue.Queue()
        # allow multiple SerialBuffers to share a line queue so you can merge
        # servo's CPU and EC streams into one thing to watch the boot/test
        # progress on.
        if line_queue:
            self.line_queue = line_queue
        else:
            self.line_queue = queue.Queue()
        self.prefix = prefix
        self.timeout = timeout
        self.sentinel = object()
        self.closing = False

        if self.dev:
            self.read_thread = threading.Thread(
                target=self.serial_read_thread_loop, daemon=True)
        else:
            self.read_thread = threading.Thread(
                target=self.serial_file_read_thread_loop, daemon=True)
        self.read_thread.start()

        self.lines_thread = threading.Thread(
            target=self.serial_lines_thread_loop, daemon=True)
        self.lines_thread.start()

    def close(self):
        self.closing = True
        if self.serial:
            self.serial.cancel_read()
        self.read_thread.join()
        self.lines_thread.join()
        if self.serial:
            self.serial.close()

    # Thread that just reads the bytes from the serial device to try to keep from
    # buffer overflowing it. If nothing is received in 1 minute, it finalizes.
    def serial_read_thread_loop(self):
        greet = "Serial thread reading from %s\n" % self.dev
        self.byte_queue.put(greet.encode())

        while not self.closing:
            try:
                b = self.serial.read()
                if len(b) == 0:
                    break
                self.byte_queue.put(b)
            except Exception as err:
                print(self.prefix + str(err))
                break
        self.byte_queue.put(self.sentinel)

    # Thread that just reads the bytes from the file of serial output that some
    # other process is appending to.
    def serial_file_read_thread_loop(self):
        greet = "Serial thread reading from %s\n" % self.filename
        self.byte_queue.put(greet.encode())

        while not self.closing:
            line = self.f.readline()
            if line:
                self.byte_queue.put(line)
            else:
                time.sleep(0.1)
        self.byte_queue.put(self.sentinel)

    # Thread that processes the stream of bytes to 1) log to stdout, 2) log to
    # file, 3) add to the queue of lines to be read by program logic

    def serial_lines_thread_loop(self):
        line = bytearray()
        while True:
            bytes = self.byte_queue.get(block=True)

            if bytes == self.sentinel:
                self.read_thread.join()
                self.line_queue.put(self.sentinel)
                break

            # Write our data to the output file if we're the ones reading from
            # the serial device
            if self.dev:
                self.f.write(bytes)
                self.f.flush()

            for b in bytes:
                line.append(b)
                if b == b'\n'[0]:
                    line = line.decode(errors="replace")

                    time = datetime.now().strftime('%y-%m-%d %H:%M:%S')
                    print("{endc}{time} {prefix}{line}".format(
                        time=time, prefix=self.prefix, line=line, endc='\033[0m'), flush=True, end='')

                    self.line_queue.put(line)
                    line = bytearray()

    def lines(self, timeout=None, phase=None):
        start_time = time.monotonic()
        while True:
            read_timeout = None
            if timeout:
                read_timeout = timeout - (time.monotonic() - start_time)
                if read_timeout <= 0:
                    print("read timeout waiting for serial during {}".format(phase))
                    self.close()
                    break

            try:
                line = self.line_queue.get(timeout=read_timeout)
            except queue.Empty:
                print("read timeout waiting for serial during {}".format(phase))
                self.close()
                break

            if line == self.sentinel:
                print("End of serial output")
                self.lines_thread.join()
                break

            yield line


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument('--dev', type=str, help='Serial device')
    parser.add_argument('--file', type=str,
                        help='Filename for serial output', required=True)
    parser.add_argument('--prefix', type=str,
                        help='Prefix for logging serial to stdout', nargs='?')

    args = parser.parse_args()

    ser = SerialBuffer(args.dev, args.file, args.prefix or "")
    for line in ser.lines():
        # We're just using this as a logger, so eat the produced lines and drop
        # them
        pass


if __name__ == '__main__':
    main()
