/*
 * size - get size of a file
 *
 *      for LAN disk, specially SAMBA, DosFindFirst take a lot of time
 *      if directory have a lot of file.  So, cache file size for one
 *      directory and look for cache.
 */

#define	INCL_DOS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef  CACHEDIR

#define MAXNAM  512

static  UCHAR   curPath[MAXNAM] = { 0 } ;
static  UCHAR   curPatt[MAXNAM] = { 0 } ;

typedef struct _SZ  *SZPTR ;

typedef struct _SZ {
    SZPTR   prev ;
    SZPTR   next ;
    PUCHAR  name ;
    ULONG   size ;
} SZREC ;

SZREC   list = { 0 } ;

static  void    reset(void)
{
    SZPTR   p ;

    if (list.next != NULL) {
        while ((p = list.next) != &list) {
            list.next = p->next ;
            free(p->name) ;
	    free(p) ;
        }
    }
    list.prev = &list ;
    list.next = &list ;
}

static  void    dump(void)
{
    SZPTR   p ;
    
    for (p = list.next ; p != &list ; p = p->next) {
        printf("%s %d\n", p->name, p->size) ;
    }
}

static  SZPTR   append(PUCHAR name, ULONG len)
{
    SZPTR   szp, prev, next ;
    PUCHAR  nmp ;

    szp = (SZPTR) malloc(sizeof(SZREC)) ;
    nmp = malloc(strlen(name) + 2) ;
    if (szp == NULL || nmp == NULL) {
        if (szp) free(szp) ;
	if (nmp) free(nmp) ;
	return NULL ;
    }
    strcpy(nmp, name) ;
    szp->name = nmp ;
    szp->size = len ;

    prev = list.prev ;
    next = &list ;
    
    szp->prev = prev ;
    szp->next = next ;
    prev->next = szp ;
    next->prev = szp ;

    return szp ;
}

static  SZPTR   search(PUCHAR name)
{
    SZPTR   p ;

    for (p = list.next ; p != &list ; p = p->next) {
        if (stricmp(name, p->name) == 0) {
	    return p ;
        }
    }
    return NULL ;
}

static  void    expand(PUCHAR path)
{
    APIRET          stat ;
    ULONG           cnt  ;
    HDIR            hnd  ;
    FILEFINDBUF3    info ;

    reset() ;

    strcpy(curPath, path) ;
    sprintf(curPatt, "%s%s", path, "*") ;

    hnd = 0xffffffff ; cnt = 1 ;

    stat = DosFindFirst(curPatt, &hnd, 0x27, &info, sizeof(info), &cnt, 1L) ;

    while (stat == 0) {
        info.achName[info.cchName] = '\0' ;
	append(info.achName, info.cbFile) ;
        stat = DosFindNext(hnd, &info, sizeof(info), &cnt) ;
    }
    DosFindClose(hnd) ;
}

static  char    drive[_MAX_DRIVE] ;
static  char    dir[_MAX_DIR]     ;
static  char    fname[_MAX_FNAME] ;
static  char    ext[_MAX_EXT]     ;
static  char    path[MAXNAM]      ;
static  char    base[MAXNAM]      ;

BOOL    filesize(PUCHAR name, PULONG len)
{
    SZPTR   ptr ;

    /*
     * split pathname to get directory name
     */
    _splitpath(name, drive, dir, fname, ext) ;
    sprintf(path, "%s%s", drive, dir) ;
    sprintf(base, "%s%s", fname, ext) ;
    
    if (strlen(path) == 0) {    /* no path prefix given */
        strcpy(path, "./") ;
    }
    
    /*
     * if 'path' was new one, expand directory
     */
    if (strcmp(path, curPath) != 0) {
        expand(path) ;
    }
    if ((ptr = search(base)) == NULL) {
        return FALSE ;
    }
    *len = ptr->size ;
    return TRUE ;
}

#else   /* non-CACHE version */

BOOL    filesize(PUCHAR name, PULONG len)
{
    APIRET          stat ;
    ULONG           cnt  ;
    HDIR            hnd  ;
    FILEFINDBUF3    info ;
    
    stat = DosFindFirst(curPatt, &hnd, 0x27, &info, sizeof(info), &cnt, 1L) ;
    if (stat != 0) {
        return FALSE ;
    }
    DosFindClose(hnd) ;

    *len = info.cbFile ;
    return TRUE ;
}

#endif  /* CACHEDIR */

#ifdef  TEST

int     main(int ac, char *av[])
{
    int     i ;
    ULONG   len ;
    
    for (i = 1 ; i < ac ; i++) {
        if (filesize(av[i], &len) != TRUE) {
	    printf("%s - not found\n", av[i]) ;
        } else {
	    printf("%s - %d\n", av[i], len) ;
	}
    }
    return 0 ;
}

#endif  /* TEST */
