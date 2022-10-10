/*
 * hambiff.h - PM version of 'biff' with Hamster, Global Definitions
 */

#ifndef _HAMBIFF_H
#define _HAMBIFF_H

#ifdef  DEBUG
#define TRACE       printf
#else
#define TRACE
#endif

/*
 * Program Names
 */

extern  UCHAR   ProgramPath[] ;
extern  UCHAR   ProgramName[] ;
extern  UCHAR   ProfilePath[] ;

/*
 * Mail Server Informations
 */

extern  UCHAR   mailHost[] ;
extern  UCHAR   mailUser[] ;
extern  UCHAR   mailPass[] ;
extern  ULONG   mailWait   ;

BOOL    biffInit(HAB hab, int ac, char *av[]) ;
void    biffSave(HAB hab) ;

BOOL    biffStart(void) ;
void    biffCheck(void) ;
void    biffFinish(void) ;

int     popBiff(PUCHAR host, PUCHAR user, PUCHAR pass) ;

/*
 * Window Messages to Control Mail Checking
 */

#define WM_BIFF_WAIT    (WM_USER + 1)
#define WM_BIFF_POLL    (WM_USER + 2)
#define WM_BIFF_MAIL    (WM_USER + 3)
#define WM_BIFF_FAIL    (WM_USER + 4)

/*
 * Bitmaps to show Mail Status
 */

extern  HBITMAP     hbmStat ;       /* current bitmap       */
extern  HBITMAP     hbmWait ;       /* when sleeping        */
extern  HBITMAP     hbmPoll ;       /* polling mail drop    */  
extern  HBITMAP     hbmMail ;       /* got mail             */
extern  HBITMAP     hbmFail ;       /* error detected       */

BOOL    bitmapLoad(HAB hab) ;
void    bitmapFree(void) ;
void    bitmapSize(PSIZEL sz) ;
void    bitmapDraw(HPS hps) ;

/*
 * Windows
 */

extern  HWND    hwndFrame ;

BOOL    windowCreate(HAB hab)  ;
void    windowDispose(HAB hab) ;
void    windowPlace(int ac, char *av[]) ;

/*
 * Dialogs
 */

BOOL    setupDialog(HWND hwndOwner) ;
BOOL    aboutDialog(HWND hwndOwner) ;

#endif  /* _HAMBIFF_H */
