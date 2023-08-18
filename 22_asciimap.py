from PIL import Image

def image_to_ascii(image_path, output_width):
    # Define the ASCII characters from light to dark
    ASCII_CHARS = "@%#*+=-:. "
    
    # Load the image
    image = Image.open(image_path)

    # Calculate the aspect ratio
    width, height = image.size
    aspect_ratio = height / float(width)
    output_height = int(output_width * aspect_ratio)

    # Resize the image based on the output dimensions
    image = image.resize((output_width, output_height))
    
    # Convert the image to grayscale
    grayscale_image = image.convert("L")
    
    # Convert each pixel to its corresponding ASCII character
    ascii_str = ""
    for y in range(output_height):
        for x in range(output_width):
            brightness = grayscale_image.getpixel((x, y))
            ascii_str += ASCII_CHARS[brightness * len(ASCII_CHARS) // 256]
        ascii_str += "\n"
    
    return ascii_str

if __name__ == "__main__":
    image_path = input("Enter the image path: ")
    output_width = int(input("Enter desired output width: "))
    ascii_result = image_to_ascii(image_path, output_width)
    print(ascii_result)

    # Optionally, you can save the ASCII art to a text file
    with open("ascii_art.txt", "w") as f:
        f.write(ascii_result)
