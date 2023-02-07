#!/usr/bin/python

import string
import struct
#import bytearray

mapFile = open("kernmap.small")
outputFile = open("kernel.map.o", "w+b")

# newFileByteArray = bytearray(newFileBytes)
# newFile.write(newFileByteArray)

for line in mapFile:
    line = line.split(" ")
    if line == ["\n"]:
        continue
    print(line)
    func = line[2]
    #print("function: " + str(len(func)) +":" + func);
    offset = int(line[1])
    #print("offset: " + str(offset))
    padding = (32 - len(func))
    if (padding < 0):
        print("FATAL ERROR")
        exit(1)
    print(struct.pack('I', offset))
    outputFile.write(struct.pack('I', offset))
    outputFile.write(bytearray(func, "ascii"))
    outputFile.write(bytearray(padding))

