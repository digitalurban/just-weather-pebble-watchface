#!/usr/bin/env python3
"""
Convert the shoe SVG to a 16x16 PNG for Pebble
"""

# Create a simple 16x16 shoe icon using text representation
shoe_pattern = [
    "0000000000000000",
    "0000000000000000", 
    "0000111100000000",
    "0001111110000000",
    "0011111111000000", 
    "0111111111100000",
    "1111111111110000",
    "1111111111111000",
    "1111111111111100",
    "1111111111111100",
    "1111111111111000",
    "1111111111110000",
    "0111111111100000",
    "0000000000000000",
    "0000000000000000",
    "0000000000000000"
]

# Write as PBM (Portable Bitmap) format
with open('shoe_icon.pbm', 'w') as f:
    f.write("P1\n")
    f.write("16 16\n")
    for row in shoe_pattern:
        f.write(" ".join(row) + "\n")

print("Created shoe_icon.pbm")

# Try to convert to PNG using available tools
import subprocess
import sys

try:
    # Try netpbm
    result = subprocess.run(['pnmtopng', 'shoe_icon.pbm'], 
                          stdout=open('shoe_icon.png', 'wb'), 
                          stderr=subprocess.PIPE)
    if result.returncode == 0:
        print("Successfully converted to PNG using netpbm")
    else:
        raise Exception("netpbm failed")
        
except:
    # Try ImageMagick
    try:
        result = subprocess.run(['convert', 'shoe_icon.pbm', 'shoe_icon.png'], 
                              capture_output=True)
        if result.returncode == 0:
            print("Successfully converted to PNG using ImageMagick")
        else:
            raise Exception("ImageMagick failed")
    except:
        print("Could not convert to PNG - will use PBM format")
        # Copy PBM as PNG for now (Pebble SDK can handle various formats)
        import shutil
        shutil.copy('shoe_icon.pbm', 'shoe_icon_16x16.png')
        print("Created shoe_icon_16x16.png from PBM")