import os
from random import shuffle

f = []
d = []

for (dirpath, dirnames, filenames) in os.walk('.'):
	for fname in filenames:
		f.append(dirpath + '/' + fname)
	for dname in dirnames:
		d.append(dirpath + '/' + dname + '/')

print(f)
print(d)

shuffle(f)
shuffle(d)

for i in range(5):

	try:
		print("reading " + f[i])
	except(IndexError):
		print("No more filenames")
		break

	try:
		fp = open(f[i], 'r')
	except(IOError):
		print("Unable to read file at " + f[i])
	tmp = fp.read()
	fp.close()




	try:
		print("writing to " + d[i])
	except(IndexError):
		print("No more dirnames")
		break

	try:
		fp = open(d[i] + "io_test_temp.txt", 'w')
	except(IOError):
		print("Unable to write test file in " + d[i])
	tmp = fp.write("This is a test file\n\nFile #" + str(i))
	fp.close()
	os.remove(d[i] + "io_test_temp.txt")
