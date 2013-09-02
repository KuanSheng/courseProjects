#!/usr/bin/python

from random import randint
import sys


if __name__=='__main__':

    k_num = sys.argv[1]

    seed_file = open("seeds", 'w')

    data_file = open("points", 'w')


    for i in range(200):
        x = randint(1,1000)
        y = randint(1,1000)

        data_file.write(str(x) + "," + str(y) +"\n")


    data_file.close()

    for i in range(int(k_num)):
        x = randint(1,1000)
        y = randint(1,1000)

        seed_file.write(str(x) + "," + str(y) +"\n")


    seed_file.close()
