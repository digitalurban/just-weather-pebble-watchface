#!/usr/bin/env python3
"""
Create a Material Design style walking person icon
"""
import zlib
import struct

def create_material_walk_png():
    # 16x16 Material Design style walking person (1 = black pixel, 0 = white pixel)
    # Based on Material Design directions_walk icon
    walk_pattern = [
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],  # Row 0
        [0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0],  # Row 1 - head
        [0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0],  # Row 2 - head
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],  # Row 3
        [0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0],  # Row 4 - torso top
        [0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0],  # Row 5 - arms spread, torso
        [0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0],  # Row 6 - torso
        [0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0],  # Row 7 - torso
        [0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0],  # Row 8 - torso bottom
        [0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0],  # Row 9 - legs start
        [0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0],  # Row 10 - legs spread
        [0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0],  # Row 11 - legs walking
        [0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0],  # Row 12 - legs extended
        [0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0],  # Row 13 - feet
        [1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0],  # Row 14 - feet extended
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]   # Row 15
    ]
    
    width = 16
    height = 16
    
    # Convert pattern to grayscale bytes (0 = black, 255 = white)
    raw_data = bytearray()
    for row in walk_pattern:
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
    with open('material_walk_icon.png', 'wb') as f:
        f.write(png_data)
    
    print("Created Material Design walking icon: material_walk_icon.png")

if __name__ == "__main__":
    create_material_walk_png()