#!/usr/bin/env python3
import sys
import time

HEADER = 0xAA
TAIL = 0x55

def make_frame(payload):
    checksum = 0
    for b in payload:
        checksum ^= b
    return bytes([HEADER, len(payload)]) + bytes(payload) + bytes([checksum, TAIL])

def main():
    for i in range(5):
        payload = [0x10 + i, 0x20 + i, 0x30 + i]
        sys.stdout.buffer.write(make_frame(payload))
        sys.stdout.buffer.flush()          # 关键：不 flush 会攒着，时间戳失真
        time.sleep(0.1)                    # 帧间隔 100ms

if __name__ == "__main__":
    main()