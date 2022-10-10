/*
 * main.c - PM version of 'biff' with Hamster, Program Entry
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_PM
#include <os2.h>

#include "hambiff.h"
#include "hamres.h"

/*
 * myname - adjust and save program names
 */

UCHAR   ProgramPath[256] ;
UCHAR   ProgramName[256] ;
UCHAR   ProfilePath[256] ;

static  void    myname(PSZ me)
{
    PUCHAR  p, last ;

    /*
     * full pathname of program
     */

    for (p = me, last = NULL ; *p ; p++) {
        if (*p == '/' || *p == '\\') {
            last = p ;
        }
    }
    if (last != NULL) {
        strcpy(ProgramPath, me) ;
    } else if (DosSearchPath(7, "PATH", me, ProgramPath, 256) != 0) {
        strcpy(ProgramPath, me) ;
    }

    /*
     * basename of program
     */

    for (p = ProgramPath, last = NULL ; *p ; p++) {
        if (*p == '/' || *p == '\\') {
            last = p ;
        }
    }
    if (last == NULL) {
        strcpy(ProgramName, ProgramPath) ;
    } else {
        strcpy(ProgramName, &last[1]) ;
    }
    if ((p = strrchr(ProgramName, '.')) != NULL) {
        *p = '\0' ;
    }

    /*
     * pathname of Profile
     */

    strcpy(ProfilePath, ProgramPath) ;

    if ((p = strrchr(ProfilePath, '.')) == NULL) {
        strcat(ProfilePath, ".ini") ;
    } else {
        strcpy(p, ".ini") ;
    }    
}

/*
 * Error Notify
 */

void    errMessage(PSZ msg)
{
    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, msg, ProgramName, 0, MB_OK) ;
}

/*
 * Control Dialog Position
 */

void    dialogAtCenter(HWND hwndDialog)
{
    SWP     posDlg ;
    SWP     posScr ;
    
    WinQueryWindowPos(HWND_DESKTOP, &posScr) ;
    WinQueryWindowPos(hwndDialog,   &posDlg) ;

    posDlg.x = (posScr.cx - posDlg.cx) / 2 ;
    posDlg.y = (posScr.cy - posDlg.cy) / 2 ;

    WinSetWindowPos(hwndDialog, NULLHANDLE,
                    posDlg.x, posDlg.y, 0, 0, SWP_MOVE) ;
}

void    dialogAtMouse(HWND hwndDialog)
{
    POINTL  pt ;
    SWP     posDlg ;
    SWP     posScr ;
    
    WinQueryPointerPos(HWND_DESKTOP, &pt)    ;
    WinQueryWindowPos(HWND_DESKTOP, &posScr) ;
    WinQueryWindowPos(hwndDialog,   &posDlg) ;

    posDlg.x = pt.x - (posDlg.cx / 10)    ;
    posDlg.y = pt.y - (posDlg.cy * 4 / 5) ;

    if (posDlg.x < 0) {
        posDlg.x = 0 ;
    }
    if (posDlg.y < 0) {
        posDlg.y = 0 ;
    }
    if ((posDlg.x + posDlg.cx) > posScr.cx) {
        posDlg.x = posScr.cx - posDlg.cx ;
    }
    if ((posDlg.y + posDlg.cy) > posScr.cy) {
        posDlg.y = posScr.cy - posDlg.cy ;
    }
    WinSetWindowPos(hwndDialog, NULLHANDLE,
                    posDlg.x, posDlg.y, 0, 0, SWP_MOVE) ;
}

void    dialogAtWindowBottom(HWND hwndDialog, HWND hwndWindow)
{
    SWP     posDlg ;
    SWP     posWin ;
    SWP     posScr ;
    
    WinQueryWindowPos(HWND_DESKTOP, &posScr) ;
    WinQueryWindowPos(hwndDialog, &posDlg) ;
    WinQueryWindowPos(hwndWindow, &posWin) ;

    posDlg.x = posWin.x ;
    posDlg.y = posWin.y ;

    WinSetWindowPos(hwndDialog, NULLHANDLE,
                    posDlg.x, posDlg.y, 0, 0, SWP_MOVE) ;
}

/*
 * main - program start here
 */

int     main(int ac, char *av[])
{
    HAB     hab  ;
    HMQ     hmq  ;
    QMSG    qmsg ;
    
    myname(av[0]) ;
    _wildcard(&ac, &av) ;

    hab = WinInitialize(0) ;
    hmq = WinCreateMsgQueue(hab, 0) ;
    
    if (biffInit(hab, ac, av) != TRUE) {
        WinDestroyMsgQueue(hmq) ;
        WinTerminate(hab) ;
	return 1 ;
    }
    
    bitmapLoad(hab)   ;
    windowCreate(hab) ;

    if (hbmStat == NULLHANDLE || hwndFrame == NULLHANDLE) {
        if (hbmStat == NULLHANDLE) {
	    errMessage("絵はどこにいったの？") ;
        } else {
            errMessage("窓はどこにあるの？") ;
        }
        windowDispose(hab) ;
        bitmapFree() ;
        WinDestroyMsgQueue(hmq) ;
        WinTerminate(hab) ;
	return 1 ;
    }
    windowPlace(ac, av) ;

    if (biffStart() != TRUE) {
        errMessage("メイルチェックスレッドが起動できない") ;
        windowDispose(hab) ;
        bitmapFree() ;
        WinDestroyMsgQueue(hmq) ;
        WinTerminate(hab) ;
	return 1 ;
    }
    
    /*
     * Start Window Processing
     */

    while (WinGetMsg(hab, &qmsg, 0, 0, 0)) {
        WinDispatchMsg(hab, &qmsg) ;
    }

    /*
     * dispose resources
     */
    
    biffFinish() ;
    
    windowDispose(hab) ;
    bitmapFree() ;
    
    WinDestroyMsgQueue(hmq) ;
    WinTerminate(hab) ;
    
    return 0 ; 
}
