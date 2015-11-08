#! /usr/bin/python
import sys
import getopt
from PIL import Image

#
#
#
def main( argv ):
	try:
		short = "hin:p:s:x:v"
		long =[ "help", "interleave", "name=", "pos=", "size=", "xscale=", "--verbose" ]
		opts, args = getopt.getopt( argv, short, long )

	except getopt.GetoptError:
		usage()
		exit(2)

	size = ( 0, 0 )
	pos = ( 0, 0 )
	name = "ICON"
	xscale = 1.0
	verbose = False
	interleave = False

	for o,a in opts:
		if o in ( "-p", "--pos" ):
			pos = eval( a )
		elif o in ( "-s", "--size" ):
			size = eval( a )
		elif o in ( "-n", "--name" ):
			name = a
		elif o in ( "-x", "--xscale" ):
			xscale = eval( a )
		elif o in ( "-v", "--verbose" ):
			verbose = True
		elif o in ( "-i", "--interleave" ):
			interleave = True
		else:
			usage()
			exit(0)

	# load image and extract required part
	source = Image.open( args[ 0 ] )
	if verbose: print "Source image size: ", source.size
	
	w = size[0] if size[0] > 0 else source.size[0]
	h = size[1] if size[1] > 0 else source.size[1]

	cropped = source.crop( ( pos[0], pos[1], pos[0]+w, pos[1]+h ) )
	if verbose: print "Cropped image size: ", cropped.size

	# rescale
	x = cropped.size[ 0 ]
	cropped = cropped.resize( ( int( x * xscale + 0.5 ), h ) )
	if verbose: print "Scaled image size: ", cropped.size

	# If output file specified, save and exit
	if len(args) > 1:
		if verbose: print "Save to %s" % ( args[1] )
		cropped.save( args[1] )
		exit( 0 )
   
   	# Convert pixels to hex
   	bytes, width, padding, height = tohex( cropped, interleave )
	print ""
	print "// generated from '%s' %s" % ( args[0], "as interleaved bytes" if interleave else "" )
	print "const uint8_t %s_%d_%d[%d*%d] = { %s };" % (name, width+padding, height, (width+padding)/8, height, ",".join(bytes))
	print ""

#
#
#
def tohex( image, interleave ):
	pixels = list( image.getdata() )
	transparent = pixels[0]
	width = image.size[0]
	padding = 8 - width % 8 if 0 != width % 8 else 0
	paddedWidth = width + padding
	height = image.size[1]

	# Separate lines of image into their own lists
	bytes = []
	for h in range(height):
		line = pixels[ h * width : h * width + width ]
		if padding:
			line = line + [ transparent for i in range( padding ) ]

		mask = 0x80	# left to right
		bits = 0x0
  		for px in line:
        		if px != transparent:
				bits = bits | mask
			mask = mask >> 1
			if 0 == mask:
				bytes.append( hex(bits) )
				mask = 0x80
				bits = 0x0

	# reorganize bytes for target format
	if interleave:
		shuffledBytes = []
		for column in range( paddedWidth/8 ):
			for line in range( height ):
				shuffledBytes.append( bytes[ line*paddedWidth/8 + column ] )
	else:
		shuffledBytes = bytes

   	return ( shuffledBytes, width, padding, height )
#
#
#
def usage():
    print """icon <opts> input (output)
    If output file specified, grabbed image is saved to file, else hex output created	
    -h\t --help\t Show this text
    -i\t --interleave\t Interleave hex output bytes for VFD
    -n\t --name\t Name used to generate variable name
    -p\t --pos\t Tuple with position of icon to extract
    -s\t --size\t Tuple with size of icon to extract
    -x\t --xscale\t Factor used to scale image on x-axis
    """

#
#
#
if __name__ == "__main__":
	main( sys.argv[1:] )
