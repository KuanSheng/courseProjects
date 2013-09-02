#!/usr/bin/env python

import sys

current_cust_id = -1
current_cust_trans_count = 0

for line in sys.stdin:
    line = line.strip()
    key, value = line.split('\t', 1)

    cust_id = int(key)
    cust_name = str(value)

    ## countting transaction number
    if current_cust_id == cust_id:
        current_cust_trans_count += 1
    else:
        if current_cust_id != -1:
            print current_cust_id, ",", cust_name, ",", current_cust_trans_count
        current_cust_trans_count = 1
        current_cust_id = cust_id

if current_cust_id == cust_id:
    print current_cust_id, ",", cust_name,",", current_cust_trans_count
