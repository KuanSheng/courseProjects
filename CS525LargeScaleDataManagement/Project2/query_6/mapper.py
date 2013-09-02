#!/usr/bin/env python

import sys

cust_lgest_trans_dict = {}

for line in sys.stdin:
     line = line.strip()
     data = line.split(',')
     
     tran_id = data[0]
     cust_id = data[1]
     tran_total = data[2]
     tran_desc = data[-1]
     ## just output the necessary data
     print cust_id,"\t",tran_id,",",tran_total,",",tran_desc 
             
