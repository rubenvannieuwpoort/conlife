import sys
import struct
from PIL import Image

def image_to_grayscale_binary(input_image_path, output_file_path):
    with Image.open(input_image_path) as img:
        img = img.convert("L")
        width, height = img.size
        pixel_data = list(img.getdata())
        with open(output_file_path, 'wb') as f:
            f.write(struct.pack('<HH', width, height))
            f.write(bytearray(pixel_data))

if __name__ == "__main__":
    image_to_grayscale_binary(sys.argv[1], "conway.raw")
