#include "common.h"

int sockfd;
int connfd;
sockinfo_t client_info;
info_t *head = NULL;

extern void usr_register(sockinfo_t tmp,msg_t buf);
extern void usr_login(sockinfo_t tmp, msg_t buf);
extern void cmd(sockinfo_t tmp,msg_t buf);
extern void group_chat_handler(sockinfo_t tmp, msg_t buf);
extern void usr_logout(sockinfo_t tmp, msg_t buf);
extern void heart(sockinfo_t tmp, msg_t buf);

extern info_t *usr_linklist_create();
extern void display_all();

void *handleclient(void *arg)
{
	//char buf[M];
	sockinfo_t tmp_sockinfo = *((sockinfo_t *)arg);//tmp client_info
	msg_t buf;
	do
	{
		bzero(&buf, sizeof(buf));
		read(tmp_sockinfo.sockfd, &buf, sizeof(buf));
		switch(buf.type)
		{
			case TYPE_REGISTER:
				printf("client ip %s jumping to register...\n", tmp_sockinfo.ip);
				usr_register(tmp_sockinfo, buf);
				/*if(NULL == head)
				{
					perror("linklist update failed...\n");
					exit(1);
				}*/
				display_all();
				break;
				
			case TYPE_LOGIN:
				printf("client ip %s jumping to login...\n", tmp_sockinfo.ip);
				usr_login(tmp_sockinfo, buf);
				break;
			
			case TYPE_CMD:
				printf("client ip %s jumping to cmd...\n", tmp_sockinfo.ip);
				cmd(tmp_sockinfo, buf);
				break;
				
			case TYPE_HEART:
				heart(tmp_sockinfo, buf);
				break;
			
			case TYPE_GROUPMSG:
				printf("client ip %s jumping to groupchat...\n", tmp_sockinfo.ip);
				group_chat_handler(tmp_sockinfo, buf);
				break;
				
			case TYPE_LOGOUT:
				printf("client ip %s jumping to logout...\n", tmp_sockinfo.ip);
				usr_logout(tmp_sockinfo, buf);
				break;
			
			case TYPE_EXIT:
				printf("client ip %s is exiting...\n", tmp_sockinfo.ip);
				break;	
				
			default:
				break;
		}
	}while(buf.type != TYPE_EXIT);		
	printf("client ip %s is EXITED!!!\n", tmp_sockinfo.ip);
	pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr;
	struct sockaddr_in clientaddr;	
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("socket");
		exit(1);
	}
	int on = 1;
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));//enable address reuse
	if(ret < 0)
	{
		perror("setsockopt");
		exit(1);
	}
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5678);
	servaddr.sin_addr.s_addr = INADDR_ANY;
	
	ret = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if(ret < 0)
	{
		perror("bind");
		exit(1);
	}
	ret = listen(sockfd, 5);
	if(ret < 0)
	{
		perror("listen");
		exit(1);
	}
	
	pthread_t tid;
	pthread_attr_t attr;
	ret = pthread_attr_init(&attr);
	if(ret != 0)
	{
		perror("pthread_attr_init");
		exit(1);
	}
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(ret != 0)
	{
		perror("accept");
		exit(1);
	}	
	
	head = usr_linklist_create();
	if(NULL == head)
	{
		printf("linklist create failed!\n");
		printf("Or no usr...\n");
	}
	display_all();
	
	while(1)
	{
		socklen_t len = sizeof(clientaddr);
		connfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
		if(connfd < 0)
		{
			perror("accept");
			//exit(1);
		}
		
		client_info.sockfd = connfd;
		inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, client_info.ip, sizeof(client_info.ip));
		time_t t;
		time(&t);
		client_info.time = ctime(&t);
		printf("client ip %s CONNECTED! connected time is %s", client_info.ip, client_info.time);
		
		
		pthread_create(&tid, NULL, handleclient, (void *)&client_info);
		if(ret != 0)
        	{
            		perror("pthread_create");
            		exit(1);
        	}	
	}
	pthread_attr_destroy(&attr);
	close(connfd);
	close(sockfd);
	return 0;
}




