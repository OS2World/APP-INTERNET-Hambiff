/*
 * window.c - PM version of 'biff' with Hamster, Manipulate Windows
 */

#include <stdio.h>
#include <stdlib.h>

#define INCL_PM
#include <os2.h>

#include "hambiff.h"
#include "hamres.h"

/*
 * Window to Use
 */

HWND    hwndFrame = NULLHANDLE ;

/*
 * context menu
 */

static  HWND        hwndPopup = NULLHANDLE ;

static  void    contextMenu(void)
{
    POINTL  pt   ;
    ULONG   opts ;

    if (hwndPopup == NULLHANDLE) {
        hwndPopup = WinLoadMenu(hwndFrame, NULLHANDLE, IDM_POPUP) ;
    }
    if (hwndPopup == NULLHANDLE) {
        TRACE("failed to load popup menu\n") ; fflush(stdout) ;
        return ;
    }

    WinQueryPointerPos(HWND_DESKTOP, &pt) ;

    opts = PU_HCONSTRAIN | PU_VCONSTRAIN |
             PU_KEYBOARD | PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 ;

    WinPopupMenu(HWND_DESKTOP, hwndFrame, hwndPopup,
                                pt.x, pt.y, IDM_HIDE, opts) ;
}

/*
 * procFrame - subclassed frame window procedure
 */

static  PFNWP   pfnFrame ;      /* original frame window procedure  */

static MRESULT EXPENTRY procFrame(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HPS     hps  ;
    PSWP    pswp ;

    switch (msg) {

    case WM_PAINT :
        hps = WinBeginPaint(hwnd, NULLHANDLE, NULL) ;
	bitmapDraw(hps)  ;
	WinEndPaint(hps) ;
	return (MRESULT) 0 ;

    case WM_SINGLESELECT :
        biffCheck() ;
        return (*pfnFrame) (hwnd, msg, mp1, mp2) ;

    case WM_MOUSEMOVE :                             /* NEW 99/01 */
        if (hbmStat == hbmMail) {                   /* NEW 99/01 */
	    biffCheck() ;                           /* NEW 99/01 */
        }                                           /* NEW 99/01 */
	return (*pfnFrame) (hwnd, msg, mp1, mp2) ;  /* NEW 99/01 */
	
    case WM_CONTEXTMENU :
        contextMenu() ;
	return (MRESULT) 0 ;

    case WM_BEGINDRAG :
        WinSendMsg(hwndFrame, WM_TRACKFRAME,
	        MPFROMSHORT(TF_MOVE | TF_SETPOINTERPOS), NULL) ;
        return (MRESULT) 0 ;

    case WM_COMMAND :
        switch (SHORT1FROMMP(mp1)) {
        case IDM_EXIT :
	    WinPostMsg(hwnd, WM_CLOSE, NULL, NULL) ;
	    return (MRESULT) 0 ;
        case IDM_HIDE :
	    WinShowWindow(hwnd, FALSE) ;
	    return (MRESULT) 0 ;
        case IDM_MOVE :
            WinSendMsg(hwndFrame, WM_TRACKFRAME,
	            MPFROMSHORT(TF_MOVE | TF_SETPOINTERPOS), NULL) ;
            return (MRESULT) 0 ;
        case IDM_SETUP :
	    setupDialog(hwnd) ;
	    return (MRESULT) 0 ;
        case IDM_ABOUT :
	    aboutDialog(hwnd) ;
	    return (MRESULT) 0 ;
        }
        return (MRESULT) 0 ;

    case WM_BIFF_WAIT :
        TRACE("proc BIFF_WAIT\n") ; fflush(stdout) ;
        hbmStat = hbmWait ;
	WinInvalidateRect(hwnd, NULL, FALSE) ;
	return (MRESULT) 0 ;

    case WM_BIFF_POLL :
        TRACE("proc BIFF_POLL\n") ; fflush(stdout) ;
        hbmStat = hbmPoll ;
	WinInvalidateRect(hwnd, NULL, FALSE) ;
        return (MRESULT) 0 ;
        
    case WM_BIFF_MAIL :
        TRACE("proc BIFF_MAIL\n") ; fflush(stdout) ;
        hbmStat = hbmMail ;
	WinInvalidateRect(hwnd, NULL, FALSE) ;
	return (MRESULT) 0 ;
	
    case WM_BIFF_FAIL :
        TRACE("proc BIFF_FAIL\n") ; fflush(stdout) ;
        hbmStat = hbmFail ;
	WinInvalidateRect(hwnd, NULL, FALSE) ;
	return (MRESULT) 0 ;

    default :
        break ;
    }
    return (*pfnFrame) (hwnd, msg, mp1, mp2) ;
}
     
/*
 * createFrame - create frame window
 */

static  void    createFrame(HAB hab)
{
    FRAMECDATA  fcd    ;

    memset(&fcd, 0, sizeof(fcd)) ;
    fcd.cb = sizeof(fcd) ;
    fcd.flCreateFlags = (FCF_TASKLIST | FCF_ICON) ;
    fcd.hmodResources = NULLHANDLE ;
    fcd.idResources   = ID_HAMBIFF ;

    hwndFrame = WinCreateWindow(
            HWND_DESKTOP,           /* Parent window handle     */
            WC_FRAME,               /* Frame Window Class       */
            ProgramName,            /* as Title                 */
            0,                      /* Window Style             */
            0, 0, 0, 0,             /* Position & size          */
            NULLHANDLE,             /* Owner Window             */
            HWND_TOP,               /* Z-Order                  */
            0,                      /* Window ID                */
            &fcd,                   /* Control Data             */
            NULL) ;                 /* Presentation Parameter   */

    if (hwndFrame == NULLHANDLE) {
        return ;
    }

    pfnFrame = WinSubclassWindow(hwndFrame, procFrame) ;

    WinSendMsg(hwndFrame, WM_SETICON,
        MPFROMP(WinLoadPointer(HWND_DESKTOP, NULLHANDLE, ID_HAMBIFF)), NULL) ;
}

/*
 * windowDispose - dispose window & related resources
 */

void    windowDispose(HAB hab)
{
    if (hwndFrame != NULLHANDLE) {
        WinDestroyWindow(hwndFrame) ;
	hwndFrame = NULLHANDLE ;
    }
}

/*
 * windowCreate - create windows
 */

BOOL    windowCreate(HAB hab)
{
    /*
     * Create Windows
     */

    createFrame(hab) ;
    
    if (hwndFrame == NULLHANDLE) {
        windowDispose(hab) ;
	return FALSE ;
    }
    return TRUE ;
}

/*
 * windowPlace - place window to specific position
 */

void    windowPlace(int ac, char *av[])
{
    POINTL  ptWin  ;
    POINTL  ptCur  ;
    SWP     swpScr ;
    SIZEL   sz     ;
    int     i      ;
    char    ch     ;

    /*
     * default position from Pointer
     */

    WinQueryPointerPos(HWND_DESKTOP, &ptCur) ;
    WinQueryWindowPos(HWND_DESKTOP, &swpScr) ;
    
    ptWin.x = ptCur.x ;
    ptWin.y = ptCur.y ;

    /*
     * change them if option specified
     */

    for (i = 1 ; i < ac ; i++) {
        if (av[i][0] != '-') {
	    continue ;
	}
	switch (ch = av[i][1]) {
	case 'x' :
	case 'X' :
	    if (av[i][2] != '\0') {
	        ptWin.x = atoi(&av[i][2]) ;
	    } else if ((i + 1) < ac) {
	        ptWin.x = atoi(av[i+=1]) ;
	    }
	    break ;
	case 'y' :
	case 'Y' :
	    if (av[i][2] != '\0') {
	        ptWin.y = atoi(&av[i][2]) ;
	    } else if ((i + 1) < ac) {
	        ptWin.y = atoi(av[i+=1]) ;
	    }
	    break ;
	case 'l' :
	case 'L' :
	case 'r' :
	case 'R' :
	    if (ch == 'l' || ch == 'L') {
                ptWin.x = 0 ;
            } else if (ch == 'r' || ch == 'R') {
	        ptWin.x = swpScr.cx ;
	    }
	    if ((ch = av[i][2]) != '\0') {
	        if (ch == 'b' || ch == 'B') {
		    ptWin.y = 0 ;
                } else if (ch == 't' || ch == 'T') {
		    ptWin.y = swpScr.cy ;
		}
	    }
	    break ;
        case 'b' :
	case 'B' :
	case 't' :
	case 'T' :
	    if (ch == 'b' || ch == 'B') {
                ptWin.y = 0 ;
            } else if (ch == 't' || ch == 'T') {
                ptWin.y = swpScr.cy ;
            }
	    if ((ch = av[i][2]) != '\0') {
	        if (ch == 'l' || ch == 'L') {
		    ptWin.x = 0 ;
                } else if (ch == 'r' || ch == 'R') {
		    ptWin.x = swpScr.cx ;
		}
	    }
	    break ;
	}
    }
    
    /*
     * Adjust window position not exceed desktop
     */
     
    bitmapSize(&sz) ;

    if (ptWin.x < 0) {
        ptWin.x = 0 ;
    }
    if ((ptWin.x + sz.cx) > swpScr.cx) {
        ptWin.x = swpScr.cx - sz.cx ;
    }
    if (ptWin.y < 0) {
        ptWin.y = 0 ;
    }
    if ((ptWin.y + sz.cy) > swpScr.cy) {
        ptWin.y = swpScr.cy - sz.cy ;
    }
    
    /*
     * finally place window
     */
     
    WinSetWindowPos(hwndFrame, HWND_TOP, ptWin.x, ptWin.y, 
        sz.cx, sz.cy, (SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER)) ;
}
