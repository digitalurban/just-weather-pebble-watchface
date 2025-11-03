#!/usr/bin/env python3
"""
Create a better shoe/footprint icon that actually looks like a shoe
"""
import zlib
import struct

def create_shoe_png():
    # 16x16 better shoe/footprint pattern (1 = black pixel, 0 = white pixel)
    # This should look more like an actual shoe/footprint
    shoe_pattern = [
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],  # Row 0
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],  # Row 1
        [0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0],  # Row 2 - toe area
        [0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0],  # Row 3 - toe area wider
        [0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0],  # Row 4 - front of foot
        [0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0],  # Row 5 - front of foot
        [0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0],  # Row 6 - mid foot
        [0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0],  # Row 7 - arch (narrower)
        [0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0],  # Row 8 - arch
        [0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0],  # Row 9 - arch
        [0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0],  # Row 10 - heel (wider)
        [0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0],  # Row 11 - heel (widest)
        [0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0],  # Row 12 - heel
        [0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0],  # Row 13 - heel back
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
    with open('shoe_icon_v2.png', 'wb') as f:
        f.write(png_data)
    
    print("Created better shoe icon: shoe_icon_v2.png")

if __name__ == "__main__":
    create_shoe_png()