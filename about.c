/*
 * about.c - PM version of 'biff' with Hamster, About Dialog
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_PM
#include <os2.h>

#include "hambiff.h"
#include "hamres.h"

/*
 * loadPict - load picture on dialog
 */

static  void    loadPict(HWND hwndDlg, HBITMAP hbm)
{
    HWND    hwndPict = WinWindowFromID(hwndDlg, IDD_APICT) ;

    TRACE("loadPict - hwndPict %08x, hbm %08x\n", hwndPict, hbm) ;
    fflush(stdout) ;

    WinSendMsg(hwndPict, SM_SETHANDLE, MPFROMLONG(hbm), NULL) ;
}

/*
 * loadDesc - load description info MLE
 */

static  void    loadDesc(HWND hwndDlg, int idx)
{
    HAB     hab  = WinQueryAnchorBlock(hwndDlg) ;
    HWND    hwnd = WinWindowFromID(hwndDlg, IDD_AMESG) ;
    UCHAR   buff[128] ;
    ULONG   len ;
    IPT     off ;
    
    WinSendMsg(hwnd, MLM_DISABLEREFRESH, NULL, NULL) ;

    WinSendMsg(hwnd, MLM_FORMAT, MPFROMSHORT(MLFIE_NOTRANS), NULL) ;
    WinSendMsg(hwnd, MLM_SETTABSTOP, MPFROMSHORT(20), NULL) ;
    
    len = (ULONG) WinSendMsg(hwnd, MLM_QUERYTEXTLENGTH, NULL, NULL) ;
    WinSendMsg(hwnd, MLM_DELETE, MPFROMLONG(0), MPFROMLONG(len)) ;
    
    WinSendMsg(hwnd, MLM_SETIMPORTEXPORT, MPFROMP(buff), MPFROMSHORT(128)) ;
    off = 0 ;

    while ((len = WinLoadString(hab, NULLHANDLE, idx, 128, buff)) > 0) {
        TRACE("loadDesc idx %d len %d\n", idx, len) ; fflush(stdout) ;
        len = (ULONG) WinSendMsg(hwnd, MLM_IMPORT, 
	                        MPFROMP(&off), MPFROMP(len)) ;
	off += len ;
        idx += 1   ;
    }

    WinSendMsg(hwnd, MLM_ENABLEREFRESH, NULL, NULL) ;
}

/*
 * There are multiple descriptions (pages) in 'ABOUT' dialog
 *      Should use 'NOTEBOOK' ?
 *
 *      0       Normal Description
 *      1       Waiting
 *      2       Polling
 *      3       Found Mail
 *      4       Error Detected
 */

#define NPAGES      5 

typedef struct _PAGE {
    HBITMAP     pict ;
    ULONG       desc ;
} PAGEREC, *PAGEPTR ;

static  PAGEREC     pages[] = {
    {   NULLHANDLE,     IDD_DESC0   } ,
    {   NULLHANDLE,     IDD_DESC1   } ,
    {   NULLHANDLE,     IDD_DESC2   } ,
    {   NULLHANDLE,     IDD_DESC3   } ,
    {   NULLHANDLE,     IDD_DESC4   }
} ;

static  int     curPage = 0 ;

static  void    pageInit(HWND hwnd)
{
    int     i ;
    
    TRACE("pageInit\n") ;
    
    pages[0].pict = hbmMail ;
    pages[1].pict = hbmWait ;
    pages[2].pict = hbmPoll ;
    pages[3].pict = hbmMail ;
    pages[4].pict = hbmFail ;

    curPage = 0 ;
    loadPict(hwnd, pages[curPage].pict) ;
    loadDesc(hwnd, pages[curPage].desc) ;
}

static  void    pageNext(HWND hwnd)
{
    TRACE("pageNext\n") ;

    curPage = (curPage + 1) % NPAGES ;
    loadPict(hwnd, pages[curPage].pict) ;
    loadDesc(hwnd, pages[curPage].desc) ;
}

/*
 * procAbout - dialog procedure for 'ABOUT' dialog
 */

static MRESULT EXPENTRY procAbout(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg) {

    case WM_INITDLG :
        dialogAtMouse(hwnd) ;
        pageInit(hwnd) ;
	return (MRESULT) 0 ;

    case WM_COMMAND :
        switch (SHORT1FROMMP(mp1)) {
	case DID_OK :
	case DID_CANCEL :
            loadPict(hwnd, NULLHANDLE)  ;   /* unlink bitmap */
	    WinDismissDlg(hwnd, DID_OK) ;
	    return (MRESULT) 0 ;
        case IDD_ANEXT :
            pageNext(hwnd) ;
	    return (MRESULT) 0 ;
        default :
	    return (MRESULT) 0 ;
        }
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2) ;
}

BOOL    aboutDialog(HWND hwndOwner)
{
    WinDlgBox(HWND_DESKTOP, hwndOwner, 
                    procAbout, NULLHANDLE, IDD_ABOUT, NULL) ;
    return TRUE ;
}

