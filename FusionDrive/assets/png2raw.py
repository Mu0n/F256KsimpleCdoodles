import struct
import sys
from PIL import Image

def save_palette_and_bitmap(image_path, palette_file, bitmap_file, chunk_width=16, chunk_height=16):
    # Open the image
    img = Image.open(image_path)

    # Ensure the image is in P mode (indexed color)
    if img.mode != 'P':
        raise ValueError("Image must be in indexed color (P mode)")

    # Get image dimensions
    img_width, img_height = img.size

    # Get the palette
    palette = img.getpalette()

    # Convert palette to blue, green, red, alpha format
    rgba_palette = []
    for i in range(0, len(palette), 3):
        red = palette[i]
        green = palette[i+1]
        blue = palette[i+2]
        alpha = 255  # PNG indexed mode typically does not have alpha, set default
        rgba_palette.append((blue, green, red, alpha))

    # Get the pixel index data (bitmap)
    pixel_data = list(img.getdata())

    # Write the palette to a separate file
    with open(palette_file, 'wb') as palette_f:
        palette_bytes = b''
        for color in rgba_palette:
            palette_bytes += struct.pack('BBBB', *color)
        palette_f.write(palette_bytes)

    # Write the bitmap (pixel index data) to a separate file
    with open(bitmap_file, 'wb') as bitmap_f:
        # Process image data chunk by chunk
        for y in range(0, img_height, chunk_height):
            for x in range(0, img_width, chunk_width):
                chunk = extract_chunk(pixel_data, img_width, img_height, x, y, chunk_width, chunk_height)
                chunk_bytes = struct.pack(f'{len(chunk)}B', *chunk)
                bitmap_f.write(chunk_bytes)

def extract_chunk(pixel_data, img_width, img_height, start_x, start_y, chunk_width, chunk_height):
    # Extracts a chunk (block of pixel indices) from the pixel data
    chunk = []
    for y in range(start_y, min(start_y + chunk_height, img_height)):
        for x in range(start_x, min(start_x + chunk_width, img_width)):
            # Compute the index in the pixel data list
            index = y * img_width + x
            chunk.append(pixel_data[index])
    return chunk

if __name__ == '__main__':
    # Example usage:
    #image_path = 'tile_set_lvl1.png'
    #palette_file = 'lvl1.pal'
    #bitmap_file = 'lvl1.set'
    #chunk_width = 16  # Example chunk width in pixels
    #chunk_height = 16  # Example chunk height in pixels
    if len(sys.argv) != 5:
        print("Usage: python tilemap_converter.py <input_file> <out_file_name> <chunk_width> <chunk_height> ")
        sys.exit(1)

    image_path = sys.argv[1]
    name = sys.argv[2]
    chunk_width = int(sys.argv[3])
    chunk_height = int(sys.argv[4])

    palette_file = f'{name}.pal';
    bitmap_file = f'{name}.bin';

    save_palette_and_bitmap(image_path, palette_file, bitmap_file, chunk_width, chunk_height)
