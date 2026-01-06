from PIL import Image

img = Image.open('img/font.png')

def tile_bits(tile_index):
    # mode 0 - 1bpp
    yield 0
    yield 0
    
    x0 = 12 * (tile_index % 16)
    y0 = 12 * (tile_index // 16)
    
    for y in range(12):
        for x in range(12):
            r = img.getpixel((x0 + x, y0 + y))[0]
            yield int(r >= 128)

    cur_bit_count = 12*12 + 2
    while cur_bit_count % 14:
        yield 0
        cur_bit_count += 1

def run():
    for tile_index in range(4*16):
        item = bytearray()
        bit_count = 0
        cur_bits = 0
        for bit in tile_bits(tile_index):
            cur_bits |= bit << bit_count
            bit_count += 1
            filled_bit_count = 6 if len(item) % 2 else 8
            if bit_count == filled_bit_count:
                item.append(cur_bits)
                cur_bits = 0
                bit_count = 0

        assert bit_count == 0, 'Bits weren\'t word-padded.'
        
        tile_index_hex = hex(tile_index)[2:]
        if len(tile_index_hex) == 1:
            tile_index_hex = '0' + tile_index_hex
        
        open(f'packed-data/ascii-{tile_index_hex}.bin', 'wb').write(item)

if __name__ == '__main__':
    run()
