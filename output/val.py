import sys
if(len(sys.argv) < 2):
    print('val.py [file]')
    sys.exit(1)
filename = sys.argv[1]

last = 0
req = []
with open(filename) as file:
    for line in file:
        line = line.split(';')
        if 'op' in line[0]:
            continue
            
        val = int(line[1].strip())
        if 'add' in line[0]:
            req += [val]
            
        elif 'dsp' in line[0]:
            dist = abs(last - val)
            minP = val
            minD = dist
            req.remove(val)
            for r in req:
                if abs(last-r) < minD:
                    minD = abs(last-r)
                    minP = r
            if(minD < dist):
                print('erro, disp {} deveria ser disp {}\n\tlast={}, {} > {}'.format(val, minP, last, dist, minD))
            last = val
    print('done')