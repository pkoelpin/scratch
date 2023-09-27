from scipy.spatial import ConvexHull
from scipy.spatial.qhull import QhullError

import numpy as np
import glob

infiles = glob.glob("./in/*.txt")

for infile in ['loads_large.txt']:
    print(infile)
    error = False
    tmp = []
    with open(infile, 'r') as f:
        for line in f:
            sline = line.split(',')
            if len(sline) < 2:
                print('ERROR: not enough columns')
                error = True
                break
            try:
                a = float(sline[0])
            except:
                print('ERROR: comverting first')
                error = True
                break
            try:
                b = float(sline[1])
            except:
                print('ERROR: comverting second')
                error = True
                break
            tmp.append([a,b])
        try:
            hull = ConvexHull(np.array(tmp))
        except QhullError:
            print("ERROR: qhull")
        except (ValueError, IndexError):
            print("ERROR: array input")
        else:
            with open(infile + '.res', 'w') as f:
                for x in sorted(hull.vertices):
                    f.write(str(x+1) + '\n')
    
