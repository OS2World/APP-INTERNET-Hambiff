/*
 * setup.c - PM version of 'biff' with Hamster, Setup Dialog
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_PM
#include <os2.h>

#include "hambiff.h"
#include "hamres.h"

/*
 * set/get data to/from dialog
 */

#define DSTRLEN 64      /* < MSTRLEN in biff.h */

static  void    setData(HWND hwnd)
{
    WinSendMsg(WinWindowFromID(hwnd, IDD_SHOST), 
                    EM_SETTEXTLIMIT, MPFROMSHORT(DSTRLEN), NULL) ;
    WinSendMsg(WinWindowFromID(hwnd, IDD_SUSER), 
                    EM_SETTEXTLIMIT, MPFROMSHORT(DSTRLEN), NULL) ;
    WinSendMsg(WinWindowFromID(hwnd, IDD_SPASS), 
                    EM_SETTEXTLIMIT, MPFROMSHORT(DSTRLEN), NULL) ;

    WinSendMsg(WinWindowFromID(hwnd, IDD_STIME), 
            SPBM_SETLIMITS, MPFROMSHORT(3600 * 6), MPFROMSHORT(1)) ;

    WinSetWindowText(WinWindowFromID(hwnd, IDD_SHOST), mailHost) ;
    WinSetWindowText(WinWindowFromID(hwnd, IDD_SUSER), mailUser) ;
    WinSetWindowText(WinWindowFromID(hwnd, IDD_SPASS), mailPass) ;

    WinSendMsg(WinWindowFromID(hwnd, IDD_STIME),
                SPBM_SETCURRENTVALUE, MPFROMSHORT(mailWait), NULL) ;
}

static  void    getData(HWND hwnd)
{
    
    WinQueryWindowText(WinWindowFromID(hwnd, IDD_SHOST), DSTRLEN, mailHost) ;
    WinQueryWindowText(WinWindowFromID(hwnd, IDD_SUSER), DSTRLEN, mailUser) ;
    WinQueryWindowText(WinWindowFromID(hwnd, IDD_SPASS), DSTRLEN, mailPass) ;
    
    WinSendMsg(WinWindowFromID(hwnd, IDD_STIME), SPBM_QUERYVALUE,
                MPFROMP(&mailWait), MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE)) ;
}

static MRESULT EXPENTRY procSetup(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg) {

    case WM_INITDLG :
        dialogAtMouse(hwnd) ;
	setData(hwnd) ;
	return (MRESULT) 0 ;

    case WM_COMMAND :
        switch (SHORT1FROMMP(mp1)) {
	case DID_OK :
	    getData(hwnd) ;
	    WinDismissDlg(hwnd, DID_OK) ;
	    return (MRESULT) 0 ;
	case DID_CANCEL :
	    WinDismissDlg(hwnd, DID_CANCEL) ;
	    return (MRESULT) 0 ;
        case IDD_SSAVE :
	    getData(hwnd) ;
	    biffSave(WinQueryAnchorBlock(hwnd)) ;
	    return (MRESULT) 0 ;
        }
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2) ;
}

BOOL    setupDialog(HWND hwndOwner)
{
    ULONG   ret ;
    
    ret = WinDlgBox(HWND_DESKTOP, hwndOwner, 
                    procSetup, NULLHANDLE, IDD_SETUP, NULL) ;

    if (ret == DID_OK) {
        return TRUE  ;
    } else {
        return FALSE ;
    }
}
