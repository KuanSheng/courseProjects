#!/usr/bin/env python
import sys

current_cust_id = -1
cust_lgest_trans_dict = {}

for line in sys.stdin:
    line = line.strip()
    key, value = line.split('\t', 1)
    data = value.split(',')

    ##print line

    ## get data
    cust_id = int(key)
    tran_id = int(data[0])
    tran_total = float(data[1])
    tran_desc = str(data[2])

    ## if the same cust_id, compare the transcation totla
    if current_cust_id == cust_id:
        lagest_tran_total = cust_lgest_trans_dict["lagest_tran_total"]
        if tran_total > lagest_tran_total:
            cust_lgest_trans_dict["lagest_tran_total"] = tran_total
            cust_lgest_trans_dict["lagest_tran_id"] = tran_id
            cust_lgest_trans_dict["lagest_tran_desc"] = tran_desc
        else:
            pass
    ## if not, try to output the current data
    else:
        if current_cust_id != -1:
            output_tran_id = cust_lgest_trans_dict["lagest_tran_id"]
            output_tran_total = cust_lgest_trans_dict["lagest_tran_total"]
            ouput_tran_desc = cust_lgest_trans_dict["lagest_tran_desc"]
            
            print current_cust_id,",",output_tran_id,",",output_tran_total,",",ouput_tran_desc
        else:
            pass
            
        cust_lgest_trans_dict["lagest_tran_total"] = tran_total
        cust_lgest_trans_dict["lagest_tran_id"] = tran_id
        cust_lgest_trans_dict["lagest_tran_desc"] = tran_desc
        current_cust_id = cust_id

## output the last one
if current_cust_id == cust_id:
    tran_id = cust_lgest_trans_dict["lagest_tran_id"]
    tran_total = cust_lgest_trans_dict["lagest_tran_total"]
    tran_desc = cust_lgest_trans_dict["lagest_tran_desc"]
    print current_cust_id,",",tran_id,",",tran_total,",",tran_desc
    
