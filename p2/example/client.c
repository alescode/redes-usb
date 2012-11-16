/*
 * rdate.c  client program for remote date program
 */
 
#include <stdio.h>



#include <rpc/rpc.h>    /* standard RPC include file */
#include "date.h"       /* this file is generated by rpcgen */

int main(int argc, char *argv[])
{
    CLIENT *cl;         /* RPC handle */
    char *server;
    long *lresult;      /* return value from bin_date_1() */
    char **sresult;     /* return value from str_date_1() */
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s hostname\n", argv[0]);
        exit(1);
    }
    
    server = argv[1];
    
    /*
     * Create client handle
     */
    
    if ((cl = clnt_create(server, DATE_PROG, DATE_VERS, "udp")) == NULL) {
        /*
         * can't establish connection with server
         */
         clnt_pcreateerror(server);
         exit(2);
    }
    
    /*
     * First call the remote procedure "bin_date".
     */
     
    if ( (lresult = bin_date_1(NULL, cl)) == NULL) {
        clnt_perror(cl, server);
        exit(3);
    }
    printf("time on host %s = %ld\n",server, *lresult);
    
    /*
     * Now call the remote procedure str_date
     */
     
    if ( (sresult = str_date_1(lresult, cl)) == NULL) {
        clnt_perror(cl, server);
        exit(4);
    }
    printf("time on host %s = %s", server, *sresult);
    
    clnt_destroy(cl);         /* done with the handle */
    exit(0);
}

