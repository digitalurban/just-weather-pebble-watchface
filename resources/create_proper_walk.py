#!/usr/bin/env python3
"""
Create a proper Material Design walking icon for Pebble
Based on Google's Material Design directions_walk icon
"""
import zlib
import struct

def create_proper_walk_icon():
    # 16x16 Material Design walking person (1 = black pixel, 0 = transparent/white)
    # This matches the actual Material Design directions_walk icon shape
    walk_pattern = [
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],  # Row 0
        [0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0],  # Row 1 - head
        [0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0],  # Row 2 - head outline
        [0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0],  # Row 3 - head
        [0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0],  # Row 4 - neck
        [0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0],  # Row 5 - arms and torso
        [0,0,1,0,0,1,1,1,0,0,0,0,0,0,0,0],  # Row 6 - left arm extended, torso
        [0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0],  # Row 7 - torso
        [0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0],  # Row 8 - torso
        [0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0],  # Row 9 - legs split
        [0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0],  # Row 10 - legs walking
        [0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0],  # Row 11 - legs extended
        [0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0],  # Row 12 - legs wide
        [0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0],  # Row 13 - feet
        [1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0],  # Row 14 - feet extended
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]   # Row 15
    ]
    
    width = 16
    height = 16
    
    # Convert pattern to grayscale bytes (255 = white background, 0 = black icon)
    raw_data = bytearray()
    for row in walk_pattern:
        row_data = bytearray([0])  # Filter type 0 (no filtering)
        for pixel in row:
            # For Pebble: 0 = black pixel for icon, 255 = white background
            row_data.append(0 if pixel == 1 else 255)
        raw_data.extend(row_data)
    
    # Compress the raw data
    compressed_data = zlib.compress(raw_data)
    
    # PNG signature
    png_data = bytearray([137, 80, 78, 71, 13, 10, 26, 10])
    
    # IHDR chunk - 8-bit grayscale
    ihdr_data = struct.pack('>IIBBBBB', width, height, 8, 0, 0, 0, 0)
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
    with open('proper_walk_icon.png', 'wb') as f:
        f.write(png_data)
    
    print("Created proper walking icon for Pebble: proper_walk_icon.png")

if __name__ == "__main__":
    create_proper_walk_icon()