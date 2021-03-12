/*
 * deldups - delete duplicated files
 *
 *      This is a front end for 'dupf' program
 *
 *      deldups search-path
 *          or
 *      deldups -f dup-list
 */

CALL RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
CALL SysLoadFuncs

/*
 * programs to use
 */

find = 'find'
dupf = 'dupf -v'
edit = 'me'
copy = 'cp'

/*
 * accept argument as search path
 */

PARSE ARG path

IF path = '' THEN
    DO
        SAY 'deldups search-path ...'
	SAY '  or'
	SAY 'deldups -f dups-list'
	EXIT
    END

/*
 * temp file to store list of duplicated files
 */

dups = SysTempFileName('D:\tmp\dup????.lst')

inuse = 0
/*
SIGNAL ON FAILURE   NAME cleanup
SIGNAL ON HALT      NAME cleanup
SIGNAL ON SYNTAX    NAME cleanup
*/

/*
 * if invoked as 'dupdel -f dup-list' then use that dup-list
 * otherwise create duplicate list
 */

PARSE VAR path flag list

IF flag = '-f' THEN
    CALL  uselist
ELSE
    CALL  makelist

/*
 * invoke editor to select deleting files
 */

"@me" dups
SAY ''
SAY 'delete marked files'

/*
 * delete marked files
 */

junk = STREAM(dups, 'C', 'OPEN')
inuse = 1
DO WHILE LINES(dups) = 1
    line = LINEIN(dups)
    PARSE VAR line mark '09'x fname
    SELECT
        WHEN mark = 'd' THEN
	    rc = SysFileDelete(fname)
        WHEN mark = 'D' THEN
	    rc = SysFileDelete(fname)
        OTHERWISE
	    rc = 1
    END
    IF rc = 0 THEN
        SAY fname
END
junk = STREAM(dups, 'C', 'CLOSE')
inuse = 0

/*
 * cleanup temp. file
 */

cleanup:
    IF inuse = 1 THEN
        junk = STREAM(dups, 'C', 'CLOSE')
    "@del" dups
    EXIT

/*
 * create duplicate list for given search-pathes
 */

makelist:
    "@"find path "-print |" dupf "> " dups
    RETURN

/*
 * use given duplicated list file
 */

uselist:
    "@"copy list dups
    RETURN

