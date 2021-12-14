#!/usr/bin/env python3
import sys
with open(sys.argv[1], 'rb') as fin:
    data = fin.read()

n = len(data)
if n > 510:
    print(f'boot block too large: {n} bytes (max 510)', file=sys.stderr)
    exit(1)

print(f'boot block is {n} bytes (max 510)', file=sys.stderr)
data = data.ljust(510, b'\0') + b'\x55\xaa'

with open(sys.argv[1], 'wb') as fout:
    fout.write(data)
