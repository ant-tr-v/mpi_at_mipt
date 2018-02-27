import sys
import time
import decimal as dec
from subprocess import run
from math import *

dec.getcontext().prec = 1000

def f(x, eps):
    x1 = x
    if x < 0:
        x1 = -x
    n = dec.Decimal(1)
    k = dec.Decimal(1)
    while k > eps:
        k *= x1 / n
        n += dec.Decimal(1)
    return n


digits = int(sys.argv[2])
eps = dec.Decimal(1)/dec.Decimal(10)**digits
x = dec.Decimal(float(sys.argv[1]))
proc = str(int(sys.argv[3]))

N  = f(x, eps)
prec = int((10*digits + 2)/3)

lines = []
with open("exp.cpp", "r") as file:
    lines = file.readlines()
    lines[4] = "#define PREC " + str(prec) +"\n"

with open("exp.cpp", "w") as file:
    for l in lines:
        print(l, file = file, end = "")

run(["mpic++", "exp.cpp", "-o", "bin/exp", "-std=c++11", "-Wall"])
time.sleep(0.01)
run(["mpirun", "-np", proc, "./bin/exp", str(x), str(N)])