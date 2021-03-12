#define	INCL_DOS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define IOSIZE  (1024 * 48)

static UCHAR    buff1[IOSIZE] ;
static UCHAR    buff2[IOSIZE] ;

extern  BOOL    optVerbose ;

BOOL    diff(PUCHAR fn1, PUCHAR fn2)
{
    FILE    *fp1 ;
    FILE    *fp2 ;
    int     st1, st2 ;
    BOOL    stat ;
    
    if (optVerbose) {
        fprintf(stderr, "comparing %s / %s ... ", fn1, fn2) ;
	fflush(stderr) ;
    }
    
    fp1 = fopen(fn1, "rb") ;
    fp2 = fopen(fn2, "rb") ;
    if (fp1 == NULL || fp2 == NULL) {
        fprintf(stderr, "failed\n") ;
        if (fp1 != NULL) fclose(fp1) ;
        if (fp2 != NULL) fclose(fp2) ;
        return(TRUE) ;
    }

    stat = TRUE ;
    
    while (feof(fp1) == 0 && feof(fp2) == 0) {
        st1 = fread(buff1, 1, IOSIZE, fp1) ;
        st2 = fread(buff2, 1, IOSIZE, fp2) ;
        if (st1 != st2) {
	    stat = FALSE ;
	    break ;
	}
	if (memcmp(buff1, buff2, st1) != 0) {
	    stat = FALSE ;
	    break ;
	}
    }
    fclose(fp1) ;
    fclose(fp2) ;

    if (optVerbose) {
        if (stat == TRUE){
	    fprintf(stderr, "identical\n") ;
        } else {
            fprintf(stderr, "differs\n") ;
        }
	fflush(stderr) ;
    }
    return(stat) ;
}

