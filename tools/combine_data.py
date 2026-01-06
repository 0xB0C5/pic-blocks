import os
from addrs import *

def run():
    filenames = sorted(os.listdir('packed-data'))

    addrs = []

    combined_data = bytearray()

    name_indices = []

    # Make room for addresses at start.
    for _ in range(2*len(filenames)):
        combined_data.append(0)

    for i, filename in enumerate(filenames):
        data = open(f'packed-data/{filename}', 'rb').read()

        addr = PACKED_DATA_START_ADDR_WORDS + len(combined_data)//2
        addrs.append(addr)

        combined_data.extend(data)

        # Word align.
        if len(combined_data) % 2:
            combined_data.append(0)

        name = filename.split('.')[0]
        if '-' in name:
            name, index = name.split('-')
            index = int(index, base=16)
        else:
            index = 0

        if index == 0:
            name_indices.append((name, i))

    # Write the addresses.
    for i,addr in enumerate(addrs):
        combined_data[2*i] = addr & 0xff
        combined_data[2*i+1] = addr >> 8

    # Write indices to a c header file.
    header_path = 'pic-game.X/packed_data.h'
    with open(header_path) as fin:
        old_header = fin.read()

    header_lines = []

    header_lines.append(f'#define PACKED_DATA_START {hex(PACKED_DATA_START_ADDR_WORDS)}\n')

    for name, index in name_indices:
        header_lines.append(f'#define DATA_INDEX_{name.upper()} {index}\n')

    header = ''.join(header_lines)
    if header != old_header:
        with open('pic-game.X/packed_data.h', 'w') as fout:
            fout.write(header)
        raise Exception('Packed data indices changed. Re-run code build.')

    open('packed_data/combined.bin', 'wb').write(combined_data)

if __name__ == '__main__':
    run()

