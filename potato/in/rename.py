import glob
import os

files = glob.glob("./in/id*")

for idx, f in enumerate(files, start=1):
    new_name = './in/{:03d}.txt'.format(idx)
    os.rename(f, new_name)