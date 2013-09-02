#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE 128     /* Size of local buffer */

main(int argc, char *argv[]){
    char in_buffer[BUFSIZE],out_buffer[BUFSIZE];
    char ID[8],current_id[8];
    char TransTotal[8];
    char total[64],sum[8];
    char *pch;
    int num=0;
    double TotalSum=0.0;
    strcpy(current_id,"NONE");
    while(fgets(in_buffer, BUFSIZE, stdin)!=NULL){//read lines form standard input
        pch=strtok(in_buffer,"\t");//point to the CustomerID
        strcpy(ID,pch);//get the CustomerID
        pch=strtok(NULL,"\n");//point to the TransTotal
        strcpy(TransTotal,pch);//get the TransTotal
        if(strcmp(ID,current_id)==0){// if current id is the same as previous, increase count, and sums the transtotal
            num++;
            TotalSum += strtod(TransTotal,&pch);
        }else{   //otherwise
            if(num>0){ //output this id and the aggregation information
                strcpy(out_buffer,current_id);
                strcat(out_buffer,",");
		sprintf(sum,"%d",num);
		strcat(out_buffer,sum);
		strcat(out_buffer,",");
                sprintf(total,"%f",TotalSum);
                strcat(out_buffer,total);
                fprintf(stdout,"%s\n",out_buffer);
            }
            strcpy(current_id,ID);
            num=1;
            TotalSum = strtod(TransTotal,&pch);
        }
    }
    //remember to send the last pair
    if(strcmp(current_id,ID)==0){
        strcpy(out_buffer,current_id);
        strcat(out_buffer,",");
	sprintf(sum,"%d",num);
	strcat(out_buffer,sum);
	strcat(out_buffer,",");
        sprintf(total,"%f",TotalSum);
        strcat(out_buffer,total);
        fprintf(stdout,"%s\n",out_buffer);
    }
    return 0;
}

