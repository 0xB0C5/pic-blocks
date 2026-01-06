import colorsys
from PIL import Image

palette = []

def greyscale(brightness):
	return (brightness,brightness,brightness)


color_saturations = [   1,   1, 0.7, 0.3]
color_values =      [0.25, 0.5,   1,   1]
def color(row, hue):
	return colorsys.hsv_to_rgb(hue, color_saturations[row], color_values[row])

row_count = 4
col_count = 16

for row in range(row_count):
	for col in range(col_count):
		if col < 2:
			brightness = 4*col + row
			palette.append(greyscale(brightness / 7))
		else:
			palette.append(color(row, (col-2)/14))

for i in range(len(palette)):
    palette[i] = tuple(int(x*255) for x in palette[i])


# Convert palette to 14-bit rrrrr ggggg bbbb
data = bytearray()

for i in range(len(palette)):
    r, g, b = palette[i]
    r >>= 3
    g >>= 3
    b >>= 4
    # Also update the palette so when we save an image of it below it's accurate.
    palette[i] = (r << 3, g << 3, b << 4)
    
    color = (r << 9) | (g << 4) | b
    data.append(color & 0xff)
    data.append(color >> 8)

open('packed-data/palette.bin', 'wb').write(data)

img = Image.new('RGB', (8*col_count, 8*row_count))


for row in range(row_count):
    for col in range(col_count):
        color = palette[row*col_count + col]
        for y in range(8*row, 8*row+8):
            for x in range(8*col, 8*col+8):
                img.putpixel((x, y), color)

img.save('palette.png')

    
    