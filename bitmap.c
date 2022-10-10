/*
 * bitmap.c - PM version of 'biff' with Hamster, Load/Free Bitmaps
 */

#include <stdio.h>
#include <stdlib.h>

#define INCL_PM
#include <os2.h>

#include "hambiff.h"
#include "hamres.h"

/*
 * Bitmaps to Load
 */

HBITMAP     hbmStat = NULLHANDLE ;      /* current bitmap       */ 
HBITMAP     hbmWait = NULLHANDLE ;      /* when sleeping        */
HBITMAP     hbmPoll = NULLHANDLE ;      /* polling mail drop    */  
HBITMAP     hbmMail = NULLHANDLE ;      /* got mail             */
HBITMAP     hbmFail = NULLHANDLE ;      /* error detected       */

/*
 * checkLoaded - check if bitmaps loaded
 */

static  BOOL    checkLoaded(void)
{
    if (hbmWait == NULLHANDLE) {
        return FALSE ;
    }
    if (hbmPoll == NULLHANDLE) {
        return FALSE ;
    }
    if (hbmMail == NULLHANDLE) {
        return FALSE ;
    }
    if (hbmFail == NULLHANDLE) {
        return FALSE ;
    }
    return TRUE ;
}

/*
 * bitmapFree - free bitmaps if exists
 */

void    bitmapFree(void)
{
    if (hbmWait != NULLHANDLE) {
        GpiDeleteBitmap(hbmWait) ;
	hbmWait = NULLHANDLE ;
    }
    if (hbmPoll != NULLHANDLE) {
        GpiDeleteBitmap(hbmPoll) ;
	hbmPoll = NULLHANDLE ;
    }
    if (hbmMail != NULLHANDLE) {
        GpiDeleteBitmap(hbmMail) ;
	hbmMail = NULLHANDLE ;
    }
    if (hbmFail != NULLHANDLE) {
        GpiDeleteBitmap(hbmFail) ;
	hbmFail = NULLHANDLE ;
    }
    hbmStat = NULLHANDLE ;
}

/*
 * loadOne - load a Bitmap
 */

static  HBITMAP loadOne(HAB hab, int id)
{
    HDC     hdc ;
    HPS     hps ;
    SIZEL   siz ;
    HBITMAP hbm ;
    
    siz.cx = siz.cy = 0 ;

    hdc = DevOpenDC(hab, OD_MEMORY, "*", 0, NULL, NULLHANDLE) ;
    hps = GpiCreatePS(hab, hdc, &siz,
                PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC) ;
    if (hdc == NULLHANDLE || hps == NULLHANDLE) {
        TRACE("loadOne - failure on DC/PS\n") ;
        if (hps != NULLHANDLE) GpiDestroyPS(hps) ;
        if (hdc != NULLHANDLE) DevCloseDC(hdc)   ;
        return NULLHANDLE ;
    }

    hbm = GpiLoadBitmap(hps, NULLHANDLE, id, 0, 0) ;
    
    GpiDestroyPS(hps) ;
    DevCloseDC(hdc) ;

    TRACE("load bitmap ID %d, HBITMAP %08x\n", id, hbm) ; fflush(stdout) ;
    
    if (hbm == NULLHANDLE) {
        TRACE("loadOne - Error %08x\n", WinGetLastError(hab)) ; fflush(stdout) ;
    }
    
    return hbm ;
}

/*
 * bitmapLoad - load bitmaps
 */

BOOL    bitmapLoad(HAB hab)
{
    hbmWait = loadOne(hab, ID_BMPWAIT) ;
    hbmPoll = loadOne(hab, ID_BMPPOLL) ;
    hbmMail = loadOne(hab, ID_BMPMAIL) ;
    hbmFail = loadOne(hab, ID_BMPFAIL) ;
    
    if (checkLoaded() != TRUE) {
        bitmapFree() ;
	return FALSE ;
    }
    hbmStat = hbmWait ; 
    return TRUE ;
}

/*
 * bitmapSize - query size of bitmap (assumes all bitmaps have same size)
 */

void    bitmapSize(PSIZEL sz)
{
    BITMAPINFOHEADER2   bmi ;

    bmi.cbFix = sizeof(bmi) ;
    GpiQueryBitmapInfoHeader(hbmWait, &bmi) ;
    sz->cx = bmi.cx ;
    sz->cy = bmi.cy ;
}

/*
 * bitmapDraw - draw current bitmap on given PS
 */

void    bitmapDraw(HPS hps)
{
    SIZEL   sz ;
    POINTL  apt[3] ;

    bitmapSize(&sz) ;

    apt[0].x = 0 ;
    apt[0].y = 0 ;
    apt[1].x = sz.cx ;
    apt[1].y = sz.cy ;
    apt[2].x = 0 ;
    apt[2].y = 0 ;
    GpiWCBitBlt(hps, hbmStat, 3, apt, ROP_SRCCOPY, 0) ;
}
