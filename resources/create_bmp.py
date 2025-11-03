#!/usr/bin/env python3
import struct

def create_simple_png():
    """Create a simple 16x16 black and white PNG"""
    
    # Simple shoe pattern (1 = black, 0 = white)
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
    
    # Create BMP format (simpler than PNG)
    width, height = 16, 16
    
    # BMP header
    bmp_header = bytearray([
        0x42, 0x4D,  # 'BM' signature
        0x46, 0x01, 0x00, 0x00,  # File size (326 bytes)
        0x00, 0x00, 0x00, 0x00,  # Reserved
        0x3E, 0x00, 0x00, 0x00,  # Offset to pixel data
        0x28, 0x00, 0x00, 0x00,  # DIB header size
        0x10, 0x00, 0x00, 0x00,  # Width (16)
        0x10, 0x00, 0x00, 0x00,  # Height (16)
        0x01, 0x00,  # Planes
        0x01, 0x00,  # Bits per pixel (1 bit = monochrome)
        0x00, 0x00, 0x00, 0x00,  # Compression
        0x08, 0x01, 0x00, 0x00,  # Image size
        0x12, 0x0B, 0x00, 0x00,  # X pixels per meter
        0x12, 0x0B, 0x00, 0x00,  # Y pixels per meter
        0x02, 0x00, 0x00, 0x00,  # Colors in palette
        0x00, 0x00, 0x00, 0x00,  # Important colors
        # Color palette (2 colors)
        0xFF, 0xFF, 0xFF, 0x00,  # White
        0x00, 0x00, 0x00, 0x00,  # Black
    ])
    
    # Convert pattern to bitmap data (1 bit per pixel, padded to 4-byte boundary)
    bitmap_data = bytearray()
    for row in reversed(shoe_pattern):  # BMP stores bottom-to-top
        byte_row = bytearray(2)  # 16 pixels = 2 bytes, padded to 4 bytes
        for i in range(16):
            if row[i]:
                byte_idx = i // 8
                bit_idx = 7 - (i % 8)
                byte_row[byte_idx] |= (1 << bit_idx)
        bitmap_data.extend(byte_row)
        bitmap_data.extend(b'\x00\x00')  # Padding to 4-byte boundary
    
    # Write BMP file
    with open('shoe_icon.bmp', 'wb') as f:
        f.write(bmp_header)
        f.write(bitmap_data)
    
    print("Created shoe_icon.bmp")

create_simple_png()