/*
 * d u p f  -  report duplicated files
 */

#define	INCL_DOS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

BOOL    optVerbose = FALSE ;
BOOL    optFormRep = TRUE  ;
BOOL    optFormLin = FALSE ;

/*
 * Make File list on Temp. file
 */

#define MAXNAME (512 - 8)

typedef struct {
    ULONG   siz ;
    ULONG   chk ;
    UCHAR   name[MAXNAME] ;
} FREC, *FPTR ;

FILE    *listFp = NULL ;
ULONG   listCnt = 0L ;
UCHAR   listName[MAXNAME] ;

BOOL    filesize(PUCHAR name, PULONG len) ;

BOOL    listInit(FILE *ifp)
{
    FREC        frec ;
    UCHAR       *p   ;
    ULONG       len  ;
    UCHAR       name[MAXNAME] ;

    /*
     * create temp. file for file list
     */

    if ((p = tempnam(getenv("TMP"), "dup")) == NULL) {
        fprintf(stderr, "failed to create list file\n") ;
        return(FALSE) ;
    }
    if ((listFp = fopen(p, "w+b")) == NULL) {    
        fprintf(stderr, "failed to create list file\n") ;
        return(FALSE) ;
    }
    strcpy(listName, p) ;

    fseek(listFp, 0L, SEEK_SET) ;
    listCnt = 0L ;

    while (fgets(name, MAXNAME, ifp) != NULL) {
        name[strlen(name) - 1] = '\0' ;
        strcpy(frec.name, name) ;

	if (filesize(name, &len) != TRUE) {        
#if 0
	    if (optVerbose) {
                fprintf(stderr, "<%s> error\n", name, frec.siz) ;
                fflush(stderr) ;
	    }
#endif
            continue ;  /* not a normal file */
	}
	frec.siz = len ;
	frec.chk = 0L  ;

        if (optVerbose) {
            fprintf(stderr, "<%s> siz %d\n", name, frec.siz) ;
	    fflush(stderr) ;
        }
	if (fwrite(&frec, sizeof(FREC), 1, listFp) != 1) {
	    fprintf(stderr, "cannot write to list file\n") ;
	    return(FALSE) ;
	}
	listCnt += 1 ;
    }
    return(TRUE) ;
}

void    listTerm(void)
{
    fclose(listFp)   ;
    unlink(listName) ;
}

FPTR    listGet(ULONG idx, FPTR fbuf)
{
    ULONG   pos  ;

    if (idx >= listCnt) {
        return(NULL) ;
    }
    pos = idx * sizeof(FREC) ;
    fseek(listFp, pos, SEEK_SET) ;

    if (fread(fbuf, sizeof(FREC), 1, listFp) != 1) {
        return(NULL) ;
    }
    return(fbuf) ;
}

BOOL    listPut(ULONG idx, FPTR fbuf)
{
    ULONG   pos  ;

    if (idx >= listCnt) {
        return(FALSE) ;
    }
    pos = idx * sizeof(FREC) ;
    fseek(listFp, pos, SEEK_SET) ;

    if (fwrite(fbuf, sizeof(FREC), 1, listFp) != 1) {
        return(FALSE) ;
    }
    return(TRUE) ;
}

/*
 * checkDup - check for duplication
 */

typedef struct _node {
    ULONG           idx  ;
    ULONG           siz  ;
    ULONG           chk  ;
    ULONG           use  ;
    struct _node    *nxt ;
    struct _node    *dup ;
} NREC, *NPTR ;

NPTR    checkRoot = NULL ;

ULONG   fileCHK(ULONG idx) ;
BOOL    fileDUP(ULONG id1, ULONG id2) ;

BOOL    checkDup(void)
{
    ULONG   i    ;
    FPTR    fptr ;
    FREC    frec ;
    NPTR    last ;
    NPTR    nptr ;
    NPTR    cptr ;
    
    last = NULL ;
    
    for (i = 0 ; (fptr = listGet(i, &frec)) != NULL ; i++) {
        if ((nptr = (NPTR) malloc(sizeof(NREC))) == NULL) {
            return(FALSE) ;
        }
	nptr->idx = i         ;
	nptr->siz = fptr->siz ;
	nptr->chk = 0L        ;
	nptr->use = 0L        ;
	nptr->nxt = NULL      ;
	nptr->dup = NULL      ;
	
	if (last == NULL) {
	    checkRoot = nptr ;
	} else {
	    last->nxt = nptr ;
	}
	last = nptr ;
    }

    for (nptr = checkRoot ; nptr != NULL ; nptr = nptr->nxt) {
        for (cptr = nptr->nxt ; cptr != NULL ; cptr = cptr->nxt) {
	    if (nptr->siz != cptr->siz) {
	        continue ;
	    }
	    if (nptr->chk == 0L) {
	        nptr->chk = fileCHK(nptr->idx) ;
	    }
	    if (cptr->chk == 0L) {
	        cptr->chk = fileCHK(cptr->idx) ;
	    }
	    if (nptr->chk != cptr->chk) {
	        continue ;
	    }
	    if (fileDUP(nptr->idx, cptr->idx) != TRUE) {
	        continue ;
	    }
            nptr->dup = cptr ;
	    break ;
        }
    }
}

ULONG   crc32(PUCHAR fname) ;

ULONG   fileCHK(ULONG idx)
{
    FREC    frec ;
    FPTR    fptr ;

    if ((fptr = listGet(idx, &frec)) == NULL) {
        return(0L) ;
    }
    return(crc32(fptr->name)) ;
}    

BOOL    diff(PUCHAR fn1, PUCHAR fn2) ;

BOOL    fileDUP(ULONG id1, ULONG id2)
{
    FREC    frec1, frec2 ;
    FPTR    fptr1, fptr2 ;

    fptr1 = listGet(id1, &frec1) ;
    fptr2 = listGet(id2, &frec2) ;
    
    if (fptr1 == NULL || fptr2 == NULL) {
        return(FALSE) ;
    }
    return(diff(fptr1->name, fptr2->name)) ;
}

/*
 * Reporting Duplicated Files
 */
 
void reportDupRep(void)
{
    FPTR    fptr ;
    FREC    frec ;
    NPTR    nptr ;
    NPTR    dptr ;
    BOOL    found = FALSE ;
    
    for (nptr = checkRoot ; nptr != NULL ; nptr = nptr->nxt) {
        if (nptr->dup == NULL || nptr->use != 0) {
            continue ;
        }
	printf("---\n") ;
	found = TRUE    ;
	
	for (dptr = nptr ; dptr != NULL ; dptr = dptr->dup) {
            if ((fptr = listGet(dptr->idx, &frec)) == NULL) {
                break ;
            }
	    printf("\t%s\n", fptr->name) ;
	    dptr->use = 1L ;
	}
    }
    if (found) {
        printf("---\n") ;
    }
}

void reportDupLin(void)
{
    FPTR    fptr ;
    FREC    frec ;
    NPTR    nptr ;
    NPTR    dptr ;
    
    for (nptr = checkRoot ; nptr != NULL ; nptr = nptr->nxt) {
        if (nptr->dup == NULL || nptr->use != 0) {
            continue ;
        }
	for (dptr = nptr ; dptr != NULL ; dptr = dptr->dup) {
            if ((fptr = listGet(dptr->idx, &frec)) == NULL) {
                break ;
            }
	    printf("%s ", fptr->name) ;
	    dptr->use = 1L ;
	}
	printf("\n") ;
    }
}

/*
 * m a i n  -  program entry
 */

main(int ac, char **av)
{
    int     i    ;

    for (i = 1 ; i < ac ; i++) {
        if (strcmp(av[i], "-?") == 0) {
	    printf("dupf [options] < file-list") ;
	    printf("\t-l - report in line\n") ;
	    printf("\t-r - report in vertical\n") ;
	    exit(0) ;
	} else if (strcmp(av[i], "-r") == 0) {
            optFormRep = TRUE  ;
            optFormLin = FALSE ;
        } else if (strcmp(av[i], "-l") == 0) {
            optFormRep = FALSE ;
            optFormLin = TRUE  ;
        } else if (strcmp(av[i], "-v") == 0) {
	    optVerbose = TRUE ;
	} else {
	    break ;
	}
    }
    
    if (listInit(stdin) != TRUE) {
        fprintf(stderr, "failed to make file list\n") ;
        exit(1) ;
    }

    checkDup() ;

    if (optFormRep) {
        reportDupRep() ;
    } else {
        reportDupLin() ;
    }

    listTerm() ;

    return(0) ;
}
