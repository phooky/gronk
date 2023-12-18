#!/usr/bin/env python
import math
print('M4')
n=60
r=80
skip=17

def go(point):
    print(f"G0 X{point[0]:.4f} Y{point[1]:.4f} F30")

points = [(r*math.sin(2*math.pi*i/n),r*math.cos(2*math.pi*i/n)) for i in range(n)]

go(points[0])
print('M3')
for i in range(1,len(points)+1):
    point = points[(i*skip)%len(points)]
    go(point)
print('M4')
