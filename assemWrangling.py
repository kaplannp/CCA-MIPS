"""
Neato script for:
taking a c file, 
compiling to MIPS,
yoinking the .text section out of the object file,
printing each 32 bit instruction in the .text file to a line in out file

Usage:
    first command line arg is the src c program.
    second command line arg is the dst.
"""
import os
import numpy as np
from sys import argv

#compile!
flags = "-mshared -mfp32 -mips1 -c"
os.system("mips-linux-gnu-gcc {} {} -o .tempObject".format(flags, argv[1]))
#yoink .text!
os.system("objcopy -I elf32-big -j .text -O binary .tempObject .tempTextSect")
#write in nicer format!
os.system("xxd .tempTextSect > .temp")
os.system("mv .temp .tempTextSect")
#write to one instruction per line format
text = np.loadtxt(".tempTextSect", dtype=np.unicode_, delimiter=' ', usecols=range(1,9))
text = text.flatten()
#np magic here. take every other element and concatenate, cause in last 
#structure, each element took two lines
text = np.char.add(text[0::2], text[1::2]) 
np.savetxt(argv[2], text, fmt='%s')
#cleanup
os.remove(".tempTextSect")
os.remove(".tempObject")
