/*
 * lstdups - create duplicated file list
 *
 *  This is a front end for 'dupf' program
 *
 *  Requires
 *      find    - GNU find, not DOS find
 *      dupf    - local command
 */

CALL RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
CALL SysLoadFuncs

/*
 * List of directories to scan
 */

scandir = ''

PARSE ARG name remain

DO WHILE name <> ''
    scandir = scandir || " " name
    PARSE VAR remain name remain
END
"@find" scandir "-print | dupf -v"
