#!/usr/bin/python
import argparse

def ParseLine(line):
  """ Parse a line of an Intel HEX file into its component parts """
  if line[0] != ':':
    raise "Bad line beginning"

  line = line.rstrip() # There is a trailing crlf or something?

  byteCount = int(line[1:3], 16)
  address =   int(line[3:7],16)
  type =      int(line[7:9],16)
  data =      line[9:-2]
  checksum =  int(line[-2:],16)

#  for c in data:
#    print "> ", c, "<"

  tupleData = []
  for i in range(0,len(data),2):
    tupleData.append(int(data[i:i+2],16))

  return type, address, byteCount, tupleData, checksum


def loadData(fileName):
  """ Super simple intel hex reader """

  headerData = []
  with open(fileName) as fp:
    for line in fp:
      type, address, byteCount, data, checksum = ParseLine(line)
      
      if(type == 0):
        if(address == len(headerData)):  #Note there are 2 chars per address
          headerData += data
        else:
          print "This hex file appears to have non-contiguous data! Please upgrade this script"
          exit(1)
      elif(type == 1):
        # type one is stop, so just stop.
        break;
      else:
        print "type not understood: ", type
        exit(1)

  return headerData


if __name__ == '__main__':
  parser = argparse.ArgumentParser('Read in an Intex hex file')
  parser.add_argument('i', help = 'location of the hex file to read')
  args = parser.parse_args()

  data = loadData(args.i)

  print("Address=%x Length=%i"%(0, len(data)/2))
