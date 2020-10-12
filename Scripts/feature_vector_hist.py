import matplotlib.pyplot as plt
import numpy as np
import glob
import math

plt.title("A1 features")
plt.tight_layout(pad=0)

glb = glob.glob('a1*.csv')

counter = 1
x = [1,2,3,4,5,6,7,8,9,10]
for name in glb:

    plt.subplot(7, math.ceil(len(glb) / 7.0), counter)
    nm = name.split('.')
    plt.title(nm[0])
    plt.ylim(0,1)
    plt.subplots_adjust(hspace=0.4)

    f = open(name,"r")
    lines = f.readlines()
    for line in lines:
        lineArr = line.split(',')

        if lineArr[0] != "a1":
            continue
        
        y = []
        for i in range(1, 11):
            y.append(float(lineArr[i]))

        plt.plot(x,y)



    counter+=1

plt.show()