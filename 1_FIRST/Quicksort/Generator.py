import sys
import random

if len(sys.argv) < 3:
    filename = "Default.txt"
    length = 100
else:
    filename = sys.argv[1]
    length = int(sys.argv[2])


with open(filename, 'w') as fp:
    for i in range(length):
        fp.write(str(int(random.random() * length)))
        fp.write(' ')
