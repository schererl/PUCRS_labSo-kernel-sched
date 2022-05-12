BLOCKSIZE = 512*8

add = []
dsp = []
import sys
with open(sys.argv[1]) as file:
    for line in file:
        if('add' in line):
                add += [int(line.split(';')[1])]
        elif('dsp' in line):
                dsp += [int(line.split(';')[1])]

dAdd = add[0]
bAdd = add[0]/BLOCKSIZE
dDsp = dsp[0]
bDsp = dsp[0]/BLOCKSIZE
for i in range(len(add)-1):
    dAdd += abs(add[i]-add[i+1])
    dDsp += abs(dsp[i]-dsp[i+1])
    bAdd += abs(add[i]-add[i+1])/BLOCKSIZE
    bDsp += abs(dsp[i]-dsp[i+1])/BLOCKSIZE

print('Noop Distance: {}'.format(dAdd))
print('SSTF Distance: {}'.format(dDsp))
print('Result: {:.2f}%'.format((100.0*dDsp)/dAdd))
print('Result^1: {:.2f}x'.format((1.0*dAdd)/dDsp))
print('Exploitation: {}%'.format(int(100 - ((100.0*dDsp)/dAdd))))

print('Noop blocks: {}'.format(bAdd))
print('SSTF blocks: {}'.format(bDsp))
print('Div blocks: {:.2f}%'.format((100.0*bDsp)/bAdd))