#!/usr/bin/env python3
"""
Create a shoe icon with outline instead of solid fill
"""
import zlib
import struct

def create_outlined_shoe_png():
    # 16x16 shoe outline pattern (1 = black pixel, 0 = white pixel)
    shoe_pattern = [
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],  # Row 0
        [0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,0],  # Row 1 - toe dots
        [0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0],  # Row 2 - toe outline
        [0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0],  # Row 3 - front outline
        [0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0],  # Row 4 - front outline
        [0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0],  # Row 5 - front outline
        [0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0],  # Row 6 - mid outline
        [0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0],  # Row 7 - arch (narrow)
        [0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0],  # Row 8 - arch
        [0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0],  # Row 9 - back outline
        [0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0],  # Row 10 - heel outline
        [0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0],  # Row 11 - heel outline
        [0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0],  # Row 12 - heel outline
        [0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0],  # Row 13 - heel bottom
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],  # Row 14
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]   # Row 15
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
    with open('outlined_shoe_icon.png', 'wb') as f:
        f.write(png_data)
    
    print("Created outlined shoe icon: outlined_shoe_icon.png")

if __name__ == "__main__":
    create_outlined_shoe_png()