import sys
import time
from subprocess import run
from math import *

digits = int(sys.argv[2])
x = float(float(sys.argv[1]))
proc = str(int(sys.argv[3]))
prec = int((10*digits + 2)/3)

lines = []
with open("exp.cpp", "r") as file:
    lines = file.readlines()
    lines[4] = "#define PREC " + str(prec) +"\n"

with open("exp.cpp", "w") as file:
    for l in lines:
        print(l, file = file, end = "")

run(["mpic++", "exp.cpp", "-o", "bin/exp", "-std=c++11", "-Wall", "-O3"])
time.sleep(0.01)
run(["mpirun", "-np", proc, "./bin/exp", str(x), str(-digits)])