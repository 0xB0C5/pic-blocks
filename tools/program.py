import serial
import serial.tools.list_ports
import argparse
import time
import hex2bin

from addrs import *

BAUD_RATE = 115200

CONFIG_INDEX_START = 0x07
CONFIG_INDEX_END   = 0x0c

build_path = r'pic-game.X/dist/default/production/pic-game.X.production.hex'
 
class Programmer:
    def __init__(self, args):
        self.port = args.port

        print('Connecting...')
        self.ser = serial.Serial(self.port, BAUD_RATE, timeout=0.1)
        self.ser.dsrdtr = True
        
        time.sleep(3)
        
        success = False
        for i in range(20):
            req = b'h\n'
            self.ser.write(req)
            self.ser.flush()
            res = self.ser.readline()
            if res == b'h\n':
                success = True
                break

        if not success:
            raise Exception('Failed to connect.')

        print('Connected to Programmer.')
        

    def send_command(self, cmd):
        #print('Sending:', cmd.decode('ascii'))
        self.ser.write(cmd + b'\n')

        while 1:
            res = self.ser.readline().rstrip()
            if len(res) == 0:
                continue

            break
    
        # print('Received:', res.decode('ascii').rstrip())
    
        return res

    def reset(self):
        res = self.send_command(b'#')
        if res != b'#':
            raise Exception(f'Bad response: {res}')

    def chip_erase(self):
        res = self.send_command(b'e')
        if res != b'e':
            raise Exception(f'Bad response: {res}')

    def read_page(self, addr):
        addr_hex = addr.to_bytes(2, byteorder='little').hex()
        res = self.send_command(b'r' + addr_hex.encode('ascii'))
        if len(res) != 1 + 2*ROW_SIZE_BYTES or not res.startswith(b'r'):
            raise Exception(f'Bad response: {res}')
        res_hex = str(res[1:], encoding='ascii')
        # Top 2 bits of each word are not stored.
        # Even though it seems they're always 0,
        # Programming guide says to ignore them,
        # so remove them.
        return remove_high_bits(bytes.fromhex(res_hex))

    def write_page(self, addr, data):
        if len(data) != ROW_SIZE_BYTES:
            raise Exception('Data has wrong length.')
        data = remove_high_bits(data)
        addr_hex = addr.to_bytes(2, byteorder='little').hex()
        data_hex = data.hex()
        args = addr_hex + data_hex
        req = b'w' + args.encode('ascii')
        res = self.send_command(req)
        if res != b'w': raise Exception(f'Bad response: {res}')

    def write_config_word(self, addr, data):
        if len(data) != 2:
            raise Exception('Data has wrong length.')
        data = remove_high_bits(data)
        
        addr_hex = addr.to_bytes(2, byteorder='little').hex()
        data_hex = data.hex()
        args = addr_hex + data_hex
        req = b'c' + args.encode('ascii')
        res = self.send_command(req)
        if res != b'c': raise Exception(f'Bad response: {res}')

    def dump_buffer(self):
        req = b'd'
        res = self.send_command(req)
        print('Dump:', res)

    def run(self):
        req = b'>'
        res = self.send_command(req)
        if res != b'>': raise Exception(f'Bad response: {res}')

    def stop(self):
        req = b'.'
        res = self.send_command(req)
        if res != b'.': raise Exception(f'Bad response: {res}')

def remove_high_bits(data):
    data = bytearray(data)
    for i in range(1, len(data), 2):
        data[i] &= 0x3f
    return bytes(data)

def add_binary_data(rom, addr, data):
    assert addr % 2 == 0, 'Addresses must be word-aligned.'
    assert len(data) % 2 == 0, 'Data must be an even number of words.'
    
    for i in range(0, len(data), 2):
        assert (data[i+1] & 0xc0) == 0, 'Data contains invalid upper bits.'
    
    for i in range(len(data)):
        assert rom[addr+i] == 0xff, 'ROM is not empty where data is added.'
        rom[addr + i] = data[i]


def main():
    parser = argparse.ArgumentParser(description='PIC24FJ32GU202 programmer')
    parser.add_argument('-p', '--port', type=str)
    parser.add_argument('-l', '--list-ports', action='store_true')

    args = parser.parse_args()

    if args.list_ports:
        available_ports = serial.tools.list_ports.comports()
        if len(available_ports) == 0:
            print('No COM ports connected.')
        else:
            print('Connected ports:')
            for port, desc, hwid in available_ports:
                print('   ', port)
                print('       ', desc)
        
        return

    build = hex2bin.read_hex(build_path)
    build = bytearray(build)
    
    packed_data = open('packed-data/combined.bin', 'rb').read()
    add_binary_data(build, PACKED_DATA_START_ADDR_WORDS * 2, packed_data)

    prg_rom = build[:PRG_ROM_SIZE_BYTES]
    padding = build[PRG_ROM_SIZE_BYTES:CONFIG_ADDR_BYTES] # These addresses are unimmplemented in the PIC.
    config = build[CONFIG_ADDR_BYTES:]

    open('build.bin', 'wb').write(build)

    assert all(b == 0xff for b in padding), 'Padding isn\'t empty!'
    assert len(config) == ROW_SIZE_BYTES, f'Expected 1 page of config, got {len(config)} bytes.'

    if args.port is None:
        return

    prg = Programmer(args)

    prg.reset()

    prg.chip_erase()

    empty_row = bytes(0xff for _ in range(ROW_SIZE_BYTES))
    empty_row = remove_high_bits(empty_row)

    for row_index in range(PRG_ROM_ROW_COUNT):
        progress = row_index * 100 // PRG_ROM_ROW_COUNT
        print(f'{progress}%...', end='\r')

        pic_addr = ROW_SIZE_WORDS*row_index
        rom_addr = ROW_SIZE_BYTES*row_index
        row_data = prg_rom[rom_addr:rom_addr+ROW_SIZE_BYTES]

        if all(d == 0xff for d in row_data):
            continue

        old_row_data = prg.read_page(pic_addr)

        assert old_row_data == empty_row, f'Chip not erased: {old_row_data.hex()}'
        
        # print('row_index:', row_index)
        # print('PIC_ADDR:', hex(pic_addr))

        prg.write_page(pic_addr, row_data)
        #prg.dump_buffer()
        new_row_data = prg.read_page(pic_addr)
        expected_row_data = remove_high_bits(row_data)
        assert expected_row_data == new_row_data, f'Bad PRG write:\nexpected={expected_row_data.hex()},\n  actual={new_row_data.hex()}'
        
    print('Writing Config...')

    # Write config
    for config_index in range(CONFIG_INDEX_START, CONFIG_INDEX_END):
        data = config[2*config_index:2*(config_index+1)]
        # print(data)
        prg.write_config_word(CONFIG_ADDR_WORDS + config_index, data)

    new_config = prg.read_page(CONFIG_ADDR_WORDS)
    new_config_data = new_config[2*CONFIG_INDEX_START:2*CONFIG_INDEX_END]
    expected_config_data = remove_high_bits(config[2*CONFIG_INDEX_START:2*CONFIG_INDEX_END])
    assert new_config_data == expected_config_data, f'Bad Config write: expected={new_config_data.hex()}, actual={expected_config_data.hex()}'

    print('Done! ')

    prg.stop()

if __name__ == '__main__':
    main()
