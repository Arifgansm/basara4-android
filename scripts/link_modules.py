#!/usr/bin/env python3
"""Link game's PS4 modules into probe image."""

import sys
from pathlib import Path

def main():
    if len(sys.argv) < 3:
        print("Usage: link_modules.py <prepared_image> <output>")
        return 1
    
    image_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2])
    
    print(f"Linking modules from {image_path} to {output_path}")
    
    return 0

if __name__ == '__main__':
    sys.exit(main())