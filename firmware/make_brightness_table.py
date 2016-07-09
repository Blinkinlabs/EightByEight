# Create a lookup table for a set of brightnesses, given an input
# bit depth, and output bit depth.
#
# Uses a square based table, so just an approximation!

import math
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-i", type=int, default=8, help="Input bit depth")
parser.add_argument("-o", type=int, default=10, help="Output bit depth")
parser.add_argument("-p", type=float, default=2, help="Power")

args = parser.parse_args()

inputBitDepth = args.i
outputBitDepth = args.o

inputScale = (1 << inputBitDepth)       # Number of input steps
outputScale = (1 << outputBitDepth)     # Number of output steps

out = open("brightness_table.h", "w")

out.write("// This table was automatically generated using 'make_brightness_table.py'\n")
out.write("// input bit depth: %i\n" % inputBitDepth)
out.write("// output bit depth: %i\n" % outputBitDepth)
out.write("\n")
out.write("#ifndef BRIGHTNESS_TABLE_H\n")
out.write("#define BRIGHTNESS_TABLE_H\n")
out.write("\n")
out.write("#define BRIGHTNESS_STEPS %i\n" % (inputScale))
out.write("\n")
out.write("const uint16_t brightnessTable[BRIGHTNESS_STEPS] = {\n")

for i in range(0, inputScale):
    brightness = int(math.pow(float(i)/inputScale,args.p) * outputScale)

    # Compress the low end of the scale to 1, so that we never show darkness when something is expecte
    if i > 0 and brightness == 0:
        brightness = 1

    # And force the last entry to be full brightness
    if i == inputScale - 1:
        brightness = (1 << outputBitDepth) - 1

    out.write("    % 6i, // %i\n" % (brightness, i));

out.write("};\n")
out.write("\n")
out.write("#endif\n")


