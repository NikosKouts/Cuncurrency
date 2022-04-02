import sys
import random

if len(sys.argv) == 2:
	with open('INPUT', 'w') as fp:
		for i in range(int(sys.argv[1])):
			fp.write(str(i))
			fp.write(' ')