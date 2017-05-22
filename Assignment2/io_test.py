import os
import sys
import getopt
from random import shuffle


def usage():
	print("Usage: python ./io_test.py [-v] [-n <iterations>]")


def main():
	try:
		opts, args = getopt.getopt(sys.argv[1:], "n:v")
	except(getopt.GetoptError):
		usage()
		sys.exit(1)

	f = []
	d = ['.']
	iter = 100
	verbose = False

	for o, a in opts:
		if (o == '-v'):
			verbose = True
		if (o == '-n'):
			iter = int(a)

	if (verbose):
		print("Gathering files and directories...")

	for (dirpath, dirnames, filenames) in os.walk('.'):
		for fname in filenames:
			f.append(dirpath + '/' + fname)
		for dname in dirnames:
			d.append(dirpath + '/' + dname + '/')

	if (verbose):
		print(f)
		print(d)

	shuffle(f)
	shuffle(d)

	for i in range(iter):
		findex = i%len(f)
		dindex = i%len(d)

		if (verbose):
			print("reading " + f[findex])

		try:
			fp = open(f[findex], 'r')
		except(IOError):
			print("Unable to read file at " + f[findex])
		tmp = fp.read()
		fp.close()


		if (verbose):
			print("writing to " + d[dindex])

		try:
			fp = open(d[dindex] + "io_test_temp.txt", 'w')
		except(IOError):
			print("Unable to write test file in " + d[dindex])
		tmp = fp.write("This is a test file\n\nFile #" + str(i))
		fp.close()
		os.remove(d[dindex] + "io_test_temp.txt")


if(__name__ == "__main__"):
	main()
