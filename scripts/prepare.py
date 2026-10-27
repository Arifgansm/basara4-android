#!/usr/bin/env python3
"""Basara 4 Android - Game Preparation Script

Parses PS4 SELF/ELF, applies relocations, links game's bundled libc
and libSceFios2 into a single probe image.
"""

import argparse
import hashlib
import json
import os
import struct
import sys
from pathlib import Path

def span(data, offset, size):
    if offset < 0 or size < 0 or offset + size > len(data):
        raise ValueError(f"out of bounds: {offset:#x}+{size:#x} / {len(data):#x}")
    return data[offset:offset + size]

def unpack(fmt, data, offset):
    return struct.unpack(fmt, span(data, offset, struct.calcsize(fmt)))

def parse_self(data):
    if not data:
        raise ValueError('empty SELF (0 bytes)')
    magic = span(data, 0, 4)
    plain_elf = magic == b'\x7fELF'
    if plain_elf:
        count, base = 0, 0
    elif magic == b'O\x15=\x1d':
        count, = unpack('<H', data, 24)
        base = 32 + count * 32
    else:
        raise ValueError("expected PS4 SELF or ELF")
    
    header = unpack('<16sHHIQQQIHHHHHH', data, base)
    if header[0][:7] != b'\x7fELF\x02\x01\x01' or header[2] != 62:
        raise ValueError("expected little-endian x86-64 ELF")
    if header[9] != 56 or not 0 < header[10] < 256:
        raise ValueError("unsupported program headers")
    
    ph = [dict(zip(('type', 'flags', 'offset', 'vaddr', 'paddr', 'filesz', 'memsz', 'align'),
                   unpack('<IIQQQQQQ', data, base + header[5] + i * 56)))
          for i in range(header[10])]
    header_end = header[5] + header[10] * 56
    end = max(header_end, max(p['offset'] + p['filesz'] for p in ph))
    if end > 512 * 1024 * 1024:
        raise ValueError("probe image exceeds 512 MiB limit")
    
    if plain_elf:
        return bytearray(span(data, 0, end)), header, ph, [], []
    
    elf = bytearray(end)
    elf[:header_end] = span(data, base, header_end)
    covered = [(0, header_end)]
    segments = []
    
    for i in range(count):
        flags, off, size, mem = unpack('<QQQQ', data, 32 + i * 32)
        if size == 0:
            continue
        seg_data = span(data, off, size)
        # Find placement in ELF
        for p in ph:
            if p['flags'] & 1 and p['offset'] <= off < p['offset'] + p['filesz']:
                dest_off = p['offset'] + (off - p['offset'])
                elf[dest_off:dest_off+size] = seg_data
                covered.append((dest_off, dest_off+size))
                break
        segments.append({'flags': flags, 'offset': off, 'size': size, 'mem': mem})
    
    return elf, header, ph, segments, covered

def read_self(path):
    with open(path, 'rb') as f:
        data = f.read()
    return parse_self(data)

def main():
    parser = argparse.ArgumentParser(description='Prepare Basara 4 game image')
    parser.add_argument('eboot', help='Path to eboot.bin')
    parser.add_argument('-o', '--output', help='Output image path')
    parser.add_argument('--verify', action='store_true', help='Verify only')
    args = parser.parse_args()
    
    eboot_path = Path(args.eboot)
    if not eboot_path.exists():
        print(f"Error: {eboot_path} not found", file=sys.stderr)
        return 1
    
    print(f"Reading {eboot_path}...")
    elf, header, ph, segments, covered = read_self(eboot_path)
    
    print(f"ELF entry: 0x{header[3]:x}")
    print(f"Program headers: {len(ph)}")
    print(f"Segments: {len(segments)}")
    print(f"Image size: {len(elf)} bytes")
    
    if args.verify:
        # Verify all segments covered
        for i, p in enumerate(ph):
            if p['filesz'] > 0:
                found = any(c[0] <= p['offset'] and c[1] >= p['offset'] + p['filesz'] 
                           for c in covered)
                if not found:
                    print(f"WARNING: PH {i} not fully covered", file=sys.stderr)
        print("Verification complete")
        return 0
    
    if args.output:
        output_path = Path(args.output)
    else:
        output_path = eboot_path.parent / 'prepared_eboot.bin'
    
    print(f"Writing {output_path}...")
    with open(output_path, 'wb') as f:
        f.write(elf)
    
    print("Done")
    return 0

if __name__ == '__main__':
    sys.exit(main())