#!/usr/bin/env python

#original = "original/BQSquare_208x120_60.yuv"
original = "original/BQSquare_208x120_60.txt"

print 'Before locking'

with open(original, 'r+') as f:
    raw_input('%s opened' % original)
    
    with open(original, 'r+') as g:
        raw_input('%s opened again(?)' % original)
    
    print('%s closed' % original)
