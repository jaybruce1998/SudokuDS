from PIL import Image
import sys

def image_to_coords(filename):
    # Open image
    img = Image.open(filename).convert("RGB")  # Convert to RGB to simplify checking
    width, height = img.size

    coords = []

    for y in range(height):
        for x in range(width):
            r, g, b = img.getpixel((x, y))
            # If not literally white
            if(r, g, b) == (0, 0, 0):
                coords.extend([y+1, x+1])

    # Format as requested
    coord_str = "{" + ", ".join(map(str, coords)) + "} "+str(len(coords))
    return coord_str

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <image_file>")
        sys.exit(1)

    filename = sys.argv[1]
    result = image_to_coords(filename)
    print(result)