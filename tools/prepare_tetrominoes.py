from PIL import Image
import os

tetromino_shapes = '''
....
####
....
....
-
#..
###
...
-
..#
###
...
-
.##.
.##.
....
-
.##
##.
...
-
.#.
###
...
-
##.
.##
...
'''
tetromino_shapes = [shape.strip().split('\n') for shape in tetromino_shapes.split('-')]

layout = '''
0000
1   
111 
  2 
222 
33  
33  
 44 
44  
 5  
555 
66  
 66 
'''

layout = layout.strip().split('\n')

tile_size = 12

def tile_bits(tile, bpp):
    mode = bpp - 1
    yield mode & 1
    yield (mode >> 1) & 1

    bit_count = 2

    for color_index in tile:
        for j in range(bpp):
            yield (color_index >> (bpp-1-j)) & 1
            bit_count += 1
           
    while bit_count % 14:
        bit_count += 1
        yield 0

def tile_bytes(tile, bpp):
    item = bytearray()
    bit_count = 0
    cur_bits = 0
    for bit in tile_bits(tile, bpp):
        cur_bits |= bit << bit_count
        bit_count += 1
        filled_bit_count = 6 if len(item) % 2 else 8
        if bit_count == filled_bit_count:
            item.append(cur_bits)
            cur_bits = 0
            bit_count = 0
    
    assert bit_count == 0, 'Bits weren\'t word-padded.'
    
    return bytes(item)

def prepare_tetrominoes(name, tetrominoes_img_name, bpp):
    tetrominoes_img_path = f'img/{tetrominoes_img_name}'

    img = Image.open(tetrominoes_img_path)

    tile_indices = {}
    tetromino_tile_data = []

    rotations = [
    #         x0,        y0,         dir_x,    dir_y 
        (          1,           1, ( 1,  0), ( 0,  1)),
        (          1, tile_size-1, ( 0, -1), ( 1,  0)),
        (tile_size-1, tile_size-1, (-1,  0), ( 0, -1)),
        (tile_size-1,           1, ( 0,  1), (-1,  0)),
    ]

    color_indices = {
        (210,202,166,255): 0,
        (0,0,0,255): 1,
    }

    tetromino_xy_data = []
    for tetromino_id, shape in enumerate(tetromino_shapes):
        xy = []
        height = len(shape)
        width = len(shape[0])

        for y in range(height):
            for x in range(width):
                if shape[y][x] == '#':
                    xy.append((x, y))

        assert len(xy) == 4, 'Tetromino has wrong number of pieces?'
        
        for rot in range(3):
            for i in range(4):
                x, y = xy[-4]
                if width == height:
                    x, y = height-1-y, x
                else:
                    # The only case where width != height is the O piece
                    x, y = 3-y, x-1
                xy.append((x, y))

        for x, y in xy:
            tetromino_xy_data.append(x)
            tetromino_xy_data.append(y)

    print(f'x y data for {name}:')
    print(tetromino_xy_data)

    tile_index_to_tetromino_id = []

    for grid_y in range(len(layout)):
        for grid_x in range(len(layout[grid_y])):
            tetromino_id = layout[grid_y][grid_x]
            if tetromino_id == ' ': continue
            tetromino_id = int(tetromino_id)

            for (x0, y0, dir_x, dir_y) in rotations:
                start_x = x0 + tile_size*grid_x
                start_y = y0 + tile_size*grid_y
                tile = []
                for ty in range(tile_size):
                    for tx in range(tile_size):
                        x = start_x + dir_x[0]*tx + dir_y[0]*ty
                        y = start_y + dir_x[1]*tx + dir_y[1]*ty
                        color = img.getpixel((x, y))
                        if color not in color_indices:
                            color_indices[color] = len(color_indices)
                        tile.append(color_indices[color])
                tile = tuple(tile)
                if tile not in tile_indices:
                    tile_indices[tile] = len(tile_indices)
                    tile_index_to_tetromino_id.append(tetromino_id)
                tetromino_tile_data.append(tile_indices[tile])

    colors_by_index = [None]*len(color_indices)
    for color in color_indices:
        colors_by_index[color_indices[color]] = color

    print(f'colors for {name}:')
    print(colors_by_index)
    print(len(colors_by_index))

    colors16 = []

    for r,g,b,a in colors_by_index:
        r >>= 3
        g >>= 2
        b >>= 3
        left = (r << 3) | (g >> 3)
        right = ((g << 5) | b) & 0xff
        colors16.append((left, right))

    print(' ', ','.join(str(b) for col16 in colors16 for b in col16))
    
    print(f'tile data for {name}')
    print(tetromino_tile_data)

    tiles_per_row = 16
    row_count = (len(tile_indices) + tiles_per_row - 1) // tiles_per_row

    # Create a preview image of the generated tiles for reference.
    out_img = Image.new('RGBA', (1 + tiles_per_row * (1 + tile_size), 1 + row_count * (1 + tile_size))) 

    for tile in tile_indices:
        index = tile_indices[tile]
        grid_y = index // tiles_per_row
        grid_x = index % tiles_per_row
        
        i = 0
        for ty in range(tile_size):
            for tx in range(tile_size):
                px = 1 + (tile_size + 1) * grid_x + tx
                py = 1 + (tile_size + 1) * grid_y + ty
                color = colors_by_index[tile[i]]
                if color == (255,255,255,255):
                    v = tile_index_to_tetromino_id[index] * 255 // 7
                    color = (255-v,0,v,255)
                out_img.putpixel((px, py), color)
                i += 1

    try:
        os.mkdir('img/out')
    except FileExistsError:
        pass # Folder already exists - OK

    out_img.save('img/out/' + tetrominoes_img_name)
    
    for tile in tile_indices:
        tile_index_hex = hex(tile_indices[tile])[2:]
        if len(tile_index_hex) == 1:
            tile_index_hex = '0' + tile_index_hex
        
        item = tile_bytes(tile, bpp)
        open(f'packed-data/tiles_{name}-{tile_index_hex}.bin', 'wb').write(item)

def prepare_single_tile(name, path, bpp):
    path = 'img/' + path
    color_indices = {
        (210,202,166,255): 0,
        (0,0,0,255): 1,
    }
    tile = []
    img = Image.open(path)
    for y in range(tile_size):
        for x in range(tile_size):
            color = img.getpixel((x, y))
            if color not in color_indices:
                color_indices[color] = len(color_indices)
            tile.append(color_indices[color])

    item = tile_bytes(tile, bpp)
    
    open(f'packed-data/{name}.bin', 'wb').write(item)
    
    colors_by_index = [None]*len(color_indices)
    for color in color_indices:
        colors_by_index[color_indices[color]] = color

    print(f'colors for {name}:')
    print(colors_by_index)
    print(len(colors_by_index))

    colors16 = []

    for r,g,b,a in colors_by_index:
        r >>= 3
        g >>= 2
        b >>= 3
        left = (r << 3) | (g >> 3)
        right = ((g << 5) | b) & 0xff
        colors16.append((left, right))

    print(' ', ','.join(str(b) for col16 in colors16 for b in col16))

def run():
    prepare_tetrominoes('m', 'm_yoga.png', 3)
    prepare_tetrominoes('d', 'd_ladders.png', 2)
    prepare_tetrominoes('j', 'j_junkyard.png', 2)
    prepare_single_tile('k', 'k_boulders.png', 3)

    print('Success!')

if __name__ == '__main__':
    run()
