#! python3
# Copy-To-Current-Working-Directory
# (c) 2019 Nicolaus Anderson
# This program copies the other files in the directory (and sub-directories) it occupies and pastes them in the folder from which this program was called.
# i.e. If this program is called in directory A, and this program resides in directory B, then everything in B (except this program) gets copied to directory A.
# Simple usage:
# $ x2cwd

# OPTIONS
# -h  Print help message.
# -v  Verbose. Print the file names to console as they are copied.
# -r  Recursively copy all directories. This is the default.
# -n  Do NOT recursively copy all directories.
# --sim  Simulate the copy (allows for printing) but don't perform it.

import sys, os, shutil, getopt

def usage():
	print("""Copy Contents To CWD

Options:
	-h      Display this help message.
	-v      Print the file names as they are copied.
	-r      Recursively copy all directories. This is the default.
	-n      Do NOT recursively copy all directories.
	--sim   Simulate the copy but do not perform it. (Useful with -v)
""")

class Object(object):
	pass

class Options:
	def __init__(self):
		self.verbose = False
		self.recursive = True
		self.simulate = False
		self.source = Object()
		self.source.directory = ""
		self.source.callFile = ""
		self.destination = ""

def copyFile( fileName, sourceDir, destinationDir, options ):
	sourceFile = os.path.join(sourceDir, fileName)
	destFile = os.path.join(destinationDir, fileName)
	if options.verbose:
		print('Copying file %s' % fileName)
		print('Source: %s' % sourceFile)
		print('Destination: %s' % destFile)
	if not options.simulate:
		if os.path.exists(destFile):
			os.remove(destFile)
		shutil.copy(sourceFile, destinationDir)	

def recursiveProcess( options ):
	for dirName, subdirList, fileList in os.walk(options.source.directory):
		if options.verbose:
			print('Found directory %s' % dirName)
		destDir = dirName.replace(options.source.directory, options.destination, 1)
		if not os.path.exists(destDir):
			os.mkdir(destDir)
		for fileName in fileList:
			if fileName == options.source.callFile:
				if options.verbose:
					print('Found call file.')
				continue
			copyFile( fileName, dirName, destDir, options )

def process( options ):
	fileList = os.listdir( options.source.directory )
	for fileName in fileList:
		filePath = os.path.join( options.source.directory, fileName )
		if not os.path.isfile(filePath):
			continue
		if fileName == options.source.callFile:
			if options.verbose:
				print('Found call file.')
			continue
		copyFile( fileName, options.source.directory, options.destination, options )

def main(argv):
	for a in argv:
		print("Arg = %s" % a)
	options = Options()
	myDir, myCallName = os.path.split(os.path.realpath(argv[0])) # Location of this program, directory B
	options.source.directory = myDir
	options.source.callFile = myCallName
	cwd = os.getcwd() # Location of the call (directory A)
	options.destination = os.path.normpath(cwd)

	if len(sys.argv) > 1:
		longOptions = ["sim"]
		try:
			opts, args = getopt.getopt(argv[1:], "hvrn", longOptions )
		except getopt.GetoptError:
			usage()
			sys.exit(2)
		for opt, arg in opts:
			print( "Opt({opt}), Arg({arg})".format(opt=opt, arg=arg) )
			if opt == "-h":
				usage()
				sys.exit(0)
			elif opt == "-v":
				options.verbose = True
				print("Verbosity enabled")
			elif opt == "-r":
				options.recursive = True
			elif opt == "-n":
				options.recursive = False
			elif opt == "--sim":
				options.simulate = True
	if options.recursive:
		recursiveProcess( options )
	else:
		process( options )


if __name__ == "__main__":
	main( sys.argv )
else:
	print("This program is intended to be used from the command line.")
