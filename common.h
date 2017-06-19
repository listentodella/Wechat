#ifndef __COMMON__
#define __COMMON__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>

#endif

#define M 1024

#define TYPE_CMD 0
#define TYPE_EXIT 1
#define TYPE_HEART 2
#define TYPE_OK 3
#define TYPE_ERR 4
#define TYPE_LOGIN 5
#define TYPE_LOGOUT 6
#define TYPE_REGISTER 7
#define TYPE_REPEAT 8
#define TYPE_GROUPMSG 9
/*
typedef struct msg
{
	int  type;
	char text[M];
}msg_t;

typedef struct info
{
	char name[32];
	char passwd[32];
	struct info *next;
}info_t;
*/

typedef struct sockinfo
{
	int sockfd;//connfd?
	char ip[64];
	char *time;
}sockinfo_t;

typedef struct info
{
	char name[32];
	char passwd[32];
	int online;
	sockinfo_t clinfo;
	struct info *next;
}info_t;

typedef struct msg
{
	int  type;
	info_t usrinfo;
	//sockinfo_t clinfo;
	char text[M];
}msg_t;





