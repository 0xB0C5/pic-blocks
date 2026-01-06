import sys

PAGE_SIZE = 64

def parse_hex(hex_data):
    res = []
    base_address = 0

    # Each part starts with a ':'
    parts = hex_data.split(':')[1:]
    for part in parts:
        byte_count = int(part[0:2], base=16)
        address = int(part[2:6], base=16)
        #print('Address:', hex(address))
        record_type = int(part[6:8], base=16)
        if byte_count > 0:
            data = bytes(
                int(part[8+2*i:10+2*i], base=16)
                for i in range(byte_count)
            )
        else:
            data = b''

        if record_type == 0:
            start = base_address + address
            end = start + byte_count
            while len(res) < end:
                res.append(0xff)
                res.append(0xff)

            # print(start, list(data))
            for i in range(byte_count):
                res[start+i] = data[i]
        elif record_type == 1:
            # end of file.
            #print('EOF')
            break
        elif record_type == 4:
            base_address = ((data[0] << 8) | data[1]) << 16;
            #print('base address =', hex(base_address))
        else:
            raise Exception(f'Unknown record type {record_type}')

    # Pad to fill page.
    while len(res) % PAGE_SIZE:
        res.append(0xff)

    return bytes(res)

def read_hex(path):
    with open(path, 'r') as fin:
        hex_data = fin.read()

    data = parse_hex(hex_data)

    return data

def dump_bin(path, data):
    with open(path, 'wb') as fout:
        fout.write(data)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('Usage: python hex2bin.py <source.hex> <dest.bin>')
        sys.exit(1)

    source_path = sys.argv[1]
    dest_path = sys.argv[2]

    data = read_hex(source_path)
    dump_bin(dest_path, data)

    print('Converted hex to bin:', source_path, '->', dest_path)
