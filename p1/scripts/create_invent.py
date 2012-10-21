import random 
import sys
import optparse

parser = optparse.OptionParser()

parser.add_option('-f', help='path del archivo de articulos', type='string', dest='filename')
parser.add_option('-o', help='path archivo a crear', type='string', dest='out')
parser.add_option('-n', help='numero de productos a crear en OUT', type='int', dest='num')

(opts, args) = parser.parse_args()

# Making sure all options appear
mandatories = ['filename', 'out', 'num']
for m in mandatories:
	if not opts.__dict__[m]:
		print "Mandatory option is missing\n"
		parser.print_help()
		exit(-1)


art_file = open(opts.filename, 'r')
new_file = open(opts.out, 'w')

art = []

for line in art_file:
	art.append(line.strip())

i = 0

while(i<opts.num):
	new_file.write(str(art[random.randint(0, len(art))]))
	new_file.write(' & ')
	new_file.write(str(random.randrange(0, 10000, 100)))
	new_file.write(' & ')
	new_file.write(str("{:.2f}".format(random.uniform(0,500))))
	new_file.write('\n')
	i += 1

new_file.close()



	
