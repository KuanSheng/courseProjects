#!/usr/bin/python

from random import randint, choice
import sys


if __name__=='__main__':

    k_num = sys.argv[1]

    seed_file = open("seeds", 'w')

    seeds = []

    for i in range(int(k_num)):
        x = randint(1,1000)
        y = randint(1,1000)

        seeds.append((x,y))        

        seed_file.write(str(x) + "," + str(y) +"\n")

    seed_file.close()



    for i in range(200):

        file_name = "points_"+str(i)+".txt"

        data_file = open(file_name, 'w')
        x = randint(1,1000)
        y = randint(1,1000)


        data_file.write("26-FEB-1987 15:17:11.20\n")

        data_file.write("\n")

        seed = choice(seeds)

        data_file.write(str(seed[0]) + "," + str(seed[1]) +"\n")

        data_file.write("\n")

        data_file.write(str(x) + "," + str(y) +"\n")


        data_file.close()
