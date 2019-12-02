#!/usr/bin/env python
#
# Convert an aruco image into a 4x4 array of 0 and 255 values


from PIL import Image
import sys
import pathlib

# Dimensions of the aruco marker
W, H = (6, 6)

if len(sys.argv) < 2:
    exit

img_dir = pathlib.Path(sys.argv[1])


def print_marker(aruco):
    '''Print the marker into a C 2d array'''
    # Beginning of marker
    print('{', end='')

    for c in range(1, 5):
        if c == 1:
            print('{', end='')
        else:
            print(' {', end='')

        for r in range(1, 5):
            val = img_aruco.getpixel((r, c))
            if r < 4:
                print(f'{val:<3}, ', end='')
            else:
                print(f'{val:<3}', end='')

                if c < 4:
                    print('},')
                else:
                    print("}", end='')

    # Marker finished
    print('},\n')


# Given a directory of marker, iterate over all of the files
for i, aruco_marker in enumerate(img_dir.iterdir()):
    img_orig = Image.open(aruco_marker)
    img_aruco = img_orig.convert('1').resize((6, 6))

    print(f'// Marker {i + 1} [{aruco_marker}]')

    # Create 4 dicts per marker. One per angle of rotation
    for rotation in range(4):
        print_marker(img_aruco)
        img_aruco = img_aruco.rotate(90)
