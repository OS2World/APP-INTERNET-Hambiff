/*
 * pop3.c - biff with POP3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <ctype.h>
#include <os2.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
 * POP Access Library
 */

ULONG   popTrace = 0 ;

#define POPTRACE_FUNCTION   0x0001
#define POPTRACE_MESSAGE    0x0002

/*
 * popDump - dump POP message
 */

void    popDump(char *title, char *str)
{
    char    *p ;
    
    printf("%s : [", title) ;
    
    for (p = str ; *p != '\0' ; p++) {
        if (*p == '\r') {
	    printf("<CR>") ;
	} else if (*p == '\n') {
	    printf("<LF>") ;
	} else if (*p == '\t') {
	    printf("<TAB>") ;
	} else {
	    putc(*p, stdout) ;
	}
    }
    printf("]\n") ; fflush(stdout) ;
}
 
/*
 * popSend - send request to POP server
 */

BOOL    popSend(int sock, char *req)
{
    int     len, n ;
    
    if (popTrace & POPTRACE_MESSAGE) {
        popDump("SEND", req) ;
    }
    
    len = strlen(req) ;
    
    while (len > 0) {
        if ((n = send(sock, req, len, 0)) < 0) {
	    return FALSE ;
	}
	req += n ;
	len -= n ;
    }
    return TRUE ;
}

/*
 * popRecv - get one line of response
 */

#define GOT_NONE    0
#define GOT_ERR     1
#define GOT_CR      2
#define GOT_LF      3

char    *popRecv(int sock, char *buff)
{
    char    *p ;
    int     stat = GOT_NONE ;
    
    if (popTrace & POPTRACE_FUNCTION) {
        printf("popRecv\n") ; fflush(stdout) ;
    }
    
    for (p = buff ; stat != GOT_ERR && stat != GOT_LF ; p++) {
        if (recv(sock, p, 1, 0) < 0) {
	    stat = GOT_ERR ;
	} else if (stat == GOT_NONE && *p == '\r') {
            stat = GOT_CR ;
        } else if (stat == GOT_CR && *p == '\n') {
            stat = GOT_LF ;
        } else {
            stat = GOT_NONE ;
        }
    }
    *p = '\0' ;

    if (popTrace & POPTRACE_MESSAGE) {
        popDump("RECV", buff) ;
    }
    
    if (stat == GOT_ERR) {
        return NULL ;
    } else {
        return buff ;
    }
}

/*
 * popCheck - check POP replies
 */

BOOL    popCheck(char *resp)
{
    if (strncmp(resp, "+OK", 3) != 0) {
        return FALSE ;
    }
    return TRUE ;
}

/*
 * popConn - connect to POP3 Server
 */

int     popConn(char *host)
{
    int     sock ;              /* socket to create & connect   */
    LONG    numHost       ;
    USHORT  numPort = 110 ;         /* for POP3 */
    struct  sockaddr_in popHost ;
    struct  hostent     *hp     ;
    char    greeting[256] ;

    if (popTrace & POPTRACE_FUNCTION) {
        printf("popConn %s\n", host) ; fflush(stdout) ;
    }
    
    if (isdigit(*host)) {
        numHost = inet_addr(host) ;
    } else if ((hp = gethostbyname(host)) != NULL) {
        numHost = *((ULONG *) hp->h_addr) ;
    } else {
        return -1 ;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	return -2 ;
    }

    memset(&popHost, 0, sizeof(popHost)) ;
    popHost.sin_family      = AF_INET ;
    popHost.sin_addr.s_addr = numHost ;
    popHost.sin_port = htons(numPort) ;    

    if (connect(sock, (struct sockaddr *) &popHost, sizeof(popHost)) != 0) {
        close(sock) ;
        return -3   ;
    }

    if (popRecv(sock, greeting) == NULL) {
        close(sock) ;
	return - 4 ;
    }
    if (popCheck(greeting) != TRUE) {
        close(sock) ;
	return -5 ;
    }

    if (popTrace & POPTRACE_FUNCTION) {
        printf("popConn %s OK\n", host) ; fflush(stdout) ;
    }

    return sock ;
}

/*
 * popAuth - autheticate
 */
 
BOOL    popAuth(int sock, char *user, char *pass)
{
    char    buff[256] ;
    
    if (popTrace & POPTRACE_FUNCTION) {
        printf("popAuth %s\n", user) ; fflush(stdout) ;
    }
    
    sprintf(buff, "USER %s\r\n", user) ;
    if (popSend(sock, buff) != TRUE) {
        return FALSE ;
    }
    if (popRecv(sock, buff) == NULL) {
        return FALSE ;
    }
    if (popCheck(buff) != TRUE) {
        return FALSE ;
    }
    
    sprintf(buff, "PASS %s\r\n", pass) ;
    if (popSend(sock, buff) != TRUE) {
        return FALSE ;
    }
    if (popRecv(sock, buff) == NULL) {
        return FALSE ;
    }
    if (popCheck(buff) != TRUE) {
        return FALSE ;
    }

    if (popTrace % POPTRACE_FUNCTION) {
        printf("popAuth %s OK\n", user) ; fflush(stdout) ;
    }

    return TRUE ;
}

/*
 * popStat - query number of message in mail drop
 */

int     popStat(int sock)
{
    char    buff[256] ;
    char    *pNum ;
    char    *p    ;
    
    if (popTrace & POPTRACE_FUNCTION) {
        printf("popStat\n") ; fflush(stdout) ;
    }
    
    if (popSend(sock, "STAT\r\n") != TRUE) {
        return -1 ;
    }
    if (popRecv(sock, buff) == NULL) {
        return -2 ;
    }
    if (popCheck(buff) != TRUE) {
        return -3 ;
    }
    
    for (pNum = &buff[3] ; *pNum != '\0' && isspace(*pNum) ; pNum++) ;
    for (p = pNum ; *p != '\0' && isdigit(*p) ; p++) ;
    
    if (p == pNum) {
        return -4 ;
    }
    *p = '\0' ;
    
    return atoi(pNum) ;
}

/*
 * popQuit - quit Pop Session
 */

BOOL    popQuit(int sock)
{
    char    buff[256] ;

    if (popTrace & POPTRACE_FUNCTION) {
        printf("popQuit\n") ; fflush(stdout) ;
    }
    
    if (popSend(sock, "QUIT\r\n") != TRUE) {
        return FALSE ;
    }
    if (popRecv(sock, buff) == NULL) {
        return FALSE ;
    }
    if (popCheck(buff) != TRUE) {
        return FALSE ;
    }
    return TRUE ;
}

/*
 * popBiff - check mail with POP3
 *
 *      returns
 *          < 0     as error
 *          = 0     no mail
 *          > 0     number of mails
 */
 
int     popBiff(PUCHAR host, PUCHAR user, PUCHAR pass)
{
    int     sock ;
    int     n    ;

    popTrace = 0 ;
    
    if ((sock = popConn(host)) < 0) {
	return -1 ;
    }
    if (popAuth(sock, user, pass) != TRUE) {
	close(sock) ;
	return -2 ;
    }
    if ((n = popStat(sock)) < 0) {
	close(sock) ;
	return -3 ;
    }
    popQuit(sock) ;
    close(sock) ;

    return n ;      /* here n >= 0, as normal status value */
}    
