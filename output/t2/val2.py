import sys
if(len(sys.argv) < 2):
    print('val.py [file]')
    sys.exit(1)
filename = sys.argv[1]

last = 0
req = []
v = {}
with open(filename) as file:
    for line in file:
        line = line.split(' ', 4)
        if 'op' in line[0]:
            continue
            
        val = int(line[3].strip())
        if 'add' in line[1]:
            req += [val]
            if(len(line)>4):
                v[val] = line[4]
            else:
                v[val] = ''
            
        elif 'dsp' in line[1]:
            dist = abs(last - val)
            minP = val
            minD = dist
            req.remove(val)
            for r in req:
                if abs(last-r) < minD:
                    minD = abs(last-r)
                    minP = r
            if(minD < dist):
                print('erro, disp {} deveria ser disp {}\n\tlast={}, {} > {}  \'{}\''.format(val, minP, last, dist, minD, v[val].strip()))
            last = val
    print('done')