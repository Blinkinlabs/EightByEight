# Create a lookup table for a set of brightnesses, given an input
# bit depth, and output bit depth.
#
# Uses a square based table, so just an approximation!

import math

inputBitDepth = 8
outputBitDepth = 13

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
    brightness = max(i>0,int(math.pow(float(i)/inputScale,2) * outputScale))
    out.write("    % 6i, // %i\n" % (brightness, i));

out.write("};\n")
out.write("\n")
out.write("#endif\n")
