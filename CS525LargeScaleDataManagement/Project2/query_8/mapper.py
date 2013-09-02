#!/usr/bin/env python

from urllib2 import *
import sys

customer_data_url = "http://users.wpi.edu/~xxiong/Customers"
##customer_data_url = "https://www.dropbox.com/s/koxjo7zqoyo93mo/Customers"

need_cust_dict = {}
need_cty_code = 5

## build the customer dict
ur = urlopen(customer_data_url)
contents = ur.readlines()
for line in contents:
    line = line.strip()
    data = line.split(',')

    cust_id = int(data[0])
    cust_name = str(data[1])
    cty_code = int(data[3])
    ## filter data by country code
    if cty_code == need_cty_code:
        need_cust_dict[cust_id] = cust_name

for line in sys.stdin:
    line = line.strip()
    data = line.split(',')

    cust_id = int(data[1])

    ## outout customers
    if cust_id in need_cust_dict.keys():
        cust_name = need_cust_dict[cust_id]
        print cust_id,"\t",cust_name
    


