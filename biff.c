/*
 * biff.c - check mail 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_PM
#include <os2.h>

#include "hambiff.h"
#include "hamres.h"

/*
 * Mail Server Informations
 */

#define MSTRLEN     128

UCHAR   mailHost[MSTRLEN] ;
UCHAR   mailUser[MSTRLEN] ;
UCHAR   mailPass[MSTRLEN] ;
ULONG   mailWait ;

/*
 * hide plain password with (quite simple) crypting
 */

static  void    encPass(PUCHAR plain, PUCHAR key, PUCHAR crypt)
{
    PUCHAR  sp, dp  ;
    int     i, klen ;
    
    if ((klen = strlen(key)) == 0) {
        strcpy(crypt, plain) ;
	return ;
    }
    for (sp = plain, dp = crypt, i = 0 ; i < MSTRLEN ; sp++, dp++, i++) {
        *dp = *sp ^ key[i % klen] ;
	if (*sp == '\0') {
	    break ;
	}
    }
}

static  void    decPass(PUCHAR crypt, PUCHAR key, PUCHAR plain)
{
    PUCHAR  sp, dp  ;
    int     i, klen ;
    
    if ((klen = strlen(key)) == 0) {
        strcpy(crypt, plain) ;
	return ;
    }
    for (sp = crypt, dp = plain, i = 0 ; i < MSTRLEN ; sp++, dp++, i++) {
        *dp = *sp ^ key[i % klen] ;
	if (*dp == '\0') {
	    break ;
	}
    }
}

/*
 * biffReady - check if ready to check mail drop
 */

BOOL    biffReady(void)
{
    if (strlen(mailHost) == 0) {
        TRACE("no mail host\n") ; fflush(stdout) ;
        return FALSE ;
    }
    if (strlen(mailUser) == 0) {
        TRACE("no mail user\n") ; fflush(stdout) ;
        return FALSE ;
    }
    if (strlen(mailPass) == 0) {
        TRACE("no mail pass\n") ; fflush(stdout) ;
        return FALSE ;
    }
    if (mailWait == 0) {
        TRACE("no mail wait\n") ; fflush(stdout) ;
        return FALSE ;
    }
    return TRUE ;
}

/*
 * biffInit - initialize mail server informations
 */

BOOL    biffInit(HAB hab, int ac, char *av[])
{
    HINI    hini ;
    int     i    ;
    UCHAR   buff[MSTRLEN] ;
    BOOL    stat ;
    ULONG   len, ret ;
    
    memset(mailHost, 0, MSTRLEN) ;
    memset(mailUser, 0, MSTRLEN) ;
    memset(mailPass, 0, MSTRLEN) ;
    mailWait = 30 ;     /* default every 30 sec */
    
    /*
     * first get mail server info. from Profile 
     */
     
    TRACE("Profile %s\n", ProfilePath) ; fflush(stdout) ;
    TRACE("Program %s\n", ProgramName) ; fflush(stdout) ;
    
    if ((hini = PrfOpenProfile(hab, ProfilePath)) != NULLHANDLE) {
        len = MSTRLEN ;
        stat = PrfQueryProfileData(hini, ProgramName, "HOST", buff, &len) ;
	if (stat == TRUE && len > 0) {
	    strcpy(mailHost, buff) ;
	}
	len = MSTRLEN ;
        stat = PrfQueryProfileData(hini, ProgramName, "USER", buff, &len) ;
	if (stat == TRUE && len > 0) {
	    strcpy(mailUser, buff) ;
	}
	len = MSTRLEN ;
        stat =PrfQueryProfileData(hini, ProgramName, "PASS", buff, &len) ;
        if (stat == TRUE && len > 0 && strlen(mailUser) > 0) {
	    decPass(buff, mailUser, mailPass) ;
        }
	len = sizeof(ULONG) ;
        PrfQueryProfileData(hini, ProgramName, "TIME", &mailWait, &len) ;
	PrfCloseProfile(hini) ;
    }

    /*
     * they may override with command line parameters (except PASS)
     */

    for (i = 1 ; i < ac ; i++) {
        if (av[i][0] != '-') {
	    continue ;
	}
	switch (av[i][1]) {
	case 'h' :
	case 'H' :
	    if (av[i][2] != '\0') {
	        strcpy(mailHost, &av[i][2]) ;
	    } else if ((i + 1) < ac) {
	        strcpy(mailHost, av[i+=1]) ;
	    }
	    break ;
	case 'u' :
	case 'U' :
	    if (av[i][2] != '\0') {
	        strcpy(mailUser, &av[i][2]) ;
		strcpy(mailPass, "") ;
	    } else if ((i + 1) < ac) {
	        strcpy(mailUser, av[i+=1]) ;
		strcpy(mailPass, "") ;
	    }
	    break ;
        case 'w' :
	case 'W' :
	    if (isdigit(av[i][2])) {
	        mailWait = atoi(&av[i][2]) ;
	    } else if ((i + 1) < ac && isdigit(av[i+1])) {
	        mailWait = atoi(av[i+=1]) ;
	    }
	    break ;
	}
    }    

    /*
     * mail server info. is not sufficient, start dialog to setup
     */
     
    while (biffReady() != TRUE) {
        if (setupDialog(NULLHANDLE) != TRUE) {
	    return FALSE ;
	}
    }

    return TRUE ;
}

/*
 * biffSave - save mail server informations into profile
 */

void    biffSave(HAB hab)
{
    HINI    hini ;
    UCHAR   buff[MSTRLEN] ;

    if ((hini = PrfOpenProfile(hab, ProfilePath)) != NULLHANDLE) {
        PrfWriteProfileData(hini, ProgramName, "HOST", mailHost, MSTRLEN) ;
        PrfWriteProfileData(hini, ProgramName, "USER", mailUser, MSTRLEN) ;
        encPass(mailPass, mailUser, buff) ;
        PrfWriteProfileData(hini, ProgramName, "PASS", buff, MSTRLEN) ;
        PrfWriteProfileData(hini, ProgramName, "TIME", &mailWait, sizeof(ULONG)) ;
        PrfCloseProfile(hini) ;
    }
}

/*
 * biffThread - thread to check mail
 */

static  BOOL    stopThread = FALSE ;
static  BOOL    doneThread = FALSE ;
static  HEV     evCheck ;

static  void    biffThread(void *arg)
{
    int     stat  ;
    ULONG   msec  ;
    ULONG   evcnt ;
    UCHAR   buff[64] ;
    
    msec = mailWait * 1000 ;
    
    WinPostMsg(hwndFrame, WM_BIFF_WAIT, NULL, NULL) ;
        
    while (1) {

        /*
	 * sleep til event notify or time-out
	 */
	 
        TRACE("biff Wait\n") ; fflush(stdout) ;
        DosResetEventSem(evCheck, &evcnt) ;
        DosWaitEventSem(evCheck, msec) ;
        if (stopThread) {
	    break ;
	}

        /*
	 * poll mail drop
	 */

        TRACE("biff Poll\n") ; fflush(stdout) ;
        WinPostMsg(hwndFrame, WM_BIFF_POLL, NULL, NULL) ;
	 
	stat = popBiff(mailHost, mailUser, mailPass) ;
        if (stopThread) {
	    break ;
	}
	
	TRACE("biff Stat %d\n", stat) ; fflush(stdout) ;

	if (stat == 0) {
	    WinPostMsg(hwndFrame, WM_BIFF_WAIT, NULL, NULL) ;
	    msec = mailWait * 1000 ;
	} else if (stat > 0) {
	    WinPostMsg(hwndFrame, WM_BIFF_MAIL, MPFROMSHORT(stat), NULL) ;
	    msec = 0xffffffff ;     /* wait inifinite   */
	} else {
	    WinPostMsg(hwndFrame, WM_BIFF_FAIL, MPFROMSHORT(stat), NULL) ;
	    msec = 0xffffffff ;     /* wait inifinite   */
	}
	if (stopThread) {
	    break ;
	}
    }
    TRACE("biff Done\n") ; fflush(stdout) ;
    doneThread = TRUE ;
}

/*
 * biffStart - start mail check thread
 */

#define STKSIZ  (1024 * 16)

BOOL    biffStart(void)
{
    if (DosCreateEventSem(NULL, &evCheck, 0, FALSE) != 0) {
        TRACE("biffStart - failed to create event semaphore\n") ;
	fflush(stdout) ;
	return FALSE ;
    }
    if (_beginthread(biffThread, NULL, STKSIZ, NULL) < 0) {
        TRACE("biffStart - failed to start biff thread\n") ;
	fflush(stdout) ;
        DosCloseEventSem(evCheck) ;
	return FALSE ;
    }
    return TRUE ;
}

/*
 * biffCheck - request to check mail drop
 */

void    biffCheck(void)
{
    DosPostEventSem(evCheck) ;
}

/*
 * biffFinish - terminate mail check thread
 */
 
void    biffFinish(void)
{
    int     retry ;
    
    stopThread = TRUE ;
    DosPostEventSem(evCheck) ;
    
    for (retry = 100 ; doneThread == FALSE && retry > 0 ; retry--) {
        DosSleep(50) ;
    }
    DosCloseEventSem(evCheck) ;
}
