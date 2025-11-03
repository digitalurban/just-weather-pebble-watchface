#!/usr/bin/env python3
"""
Create a simple PNG file for the shoe icon using raw PNG format
"""
import zlib
import struct

def create_png():
    # 16x16 shoe pattern (1 = black pixel, 0 = white pixel)
    shoe_pattern = [
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], 
        [0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0],
        [0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0],
        [0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0], 
        [0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0],
        [1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0],
        [1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0],
        [1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0],
        [1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0],
        [1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0],
        [1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0],
        [0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    ]
    
    width = 16
    height = 16
    
    # Convert pattern to grayscale bytes (0 = black, 255 = white)
    raw_data = bytearray()
    for row in shoe_pattern:
        row_data = bytearray([0])  # Filter type 0 (no filtering)
        for pixel in row:
            row_data.append(255 if pixel == 0 else 0)  # 0=white, 1=black -> 255=white, 0=black
        raw_data.extend(row_data)
    
    # Compress the raw data
    compressed_data = zlib.compress(raw_data)
    
    # PNG signature
    png_data = bytearray([137, 80, 78, 71, 13, 10, 26, 10])
    
    # IHDR chunk
    ihdr_data = struct.pack('>IIBBBBB', width, height, 8, 0, 0, 0, 0)  # 8-bit grayscale
    ihdr_crc = zlib.crc32(b'IHDR' + ihdr_data) & 0xffffffff
    png_data.extend(struct.pack('>I', len(ihdr_data)))
    png_data.extend(b'IHDR')
    png_data.extend(ihdr_data)
    png_data.extend(struct.pack('>I', ihdr_crc))
    
    # IDAT chunk
    idat_crc = zlib.crc32(b'IDAT' + compressed_data) & 0xffffffff
    png_data.extend(struct.pack('>I', len(compressed_data)))
    png_data.extend(b'IDAT')
    png_data.extend(compressed_data)
    png_data.extend(struct.pack('>I', idat_crc))
    
    # IEND chunk
    iend_crc = zlib.crc32(b'IEND') & 0xffffffff
    png_data.extend(struct.pack('>I', 0))
    png_data.extend(b'IEND')
    png_data.extend(struct.pack('>I', iend_crc))
    
    # Write PNG file
    with open('shoe_icon.png', 'wb') as f:
        f.write(png_data)
    
    print("Created proper PNG: shoe_icon.png")

if __name__ == "__main__":
    create_png()