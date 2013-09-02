#include <stdio.h>
#include <string.h>

#define BUFSIZE 1024     /* Size of local buffer */

main(int argc, char *argv[]){
    char in_buffer[BUFSIZE],out_buffer[16];
    char *pch;
    while(fgets(in_buffer, BUFSIZE, stdin)!=NULL){//get lines from standard inpuut
        pch=strtok(in_buffer,",\n");//point to the TransID
        pch=strtok(NULL,",\n");//point to the CustomerID
              strcpy(out_buffer,pch);//get the CustomerID
              strcat(out_buffer,"\t");//set the separator
        pch=strtok(NULL,",\n");//point to the TransTotal
          strcat(out_buffer,pch);//get the TransTotal
        fprintf(stdout, "%s\n", out_buffer);//write to standard output
    }
    return 0;
}

