import os
import shutil

import prepare_tetrominoes
import ascii
import combine_data

try:
    shutil.rmtree('packed-data')
except FileNotFoundError:
    pass # Already deleted - OK

os.mkdir('packed-data')

prepare_tetrominoes.run()
ascii.run()
combine_data.run()
