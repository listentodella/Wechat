#include "common.h"

extern int sockfd;
extern sigset_t sigmask;
char usr_name[32];

/*void single_chat()
{
	printf("要发送给谁？\n");
}
*/

void group_chat()
{
	sleep(1);
	system("clear");
	printf("\t\t\t###############群聊###############\n");
	msg_t send, recv;
	send.type = TYPE_GROUPMSG;
	strcpy(send.usrinfo.name, usr_name);
	
	int ret=write(sockfd, &send,sizeof(send));//去触发服务器的群聊函数
    	if(ret < 0)
    	{
        	perror("write GROUPMSG");
        	exit(0);
    	}

    	fd_set readfds;
   
    	while(1)
    	{
        	printf("\033[32m");//字色 30~39,设置绿色为发信
        	printf("~x~x~x~x~x~x~x~x~x~x~x~x~x~\n");
        	printf("\033[0m");

        	FD_ZERO(&readfds);//清空set集合
        	FD_SET(0, &readfds);//添加fd到set集合中,0是标准输入/键盘
        	FD_SET(sockfd, &readfds);//添加sockfd到set集合中
        	//ret = select(sockfd + 1, &readfds, NULL, NULL, NULL);//select IO复用,判断哪些客户端对应的sockfd就绪.  3个NULL依次是可写,异常,阻塞
        								//无法轻易抵挡signal的摧残
        	ret = pselect(sockfd + 1, &readfds, NULL, NULL, NULL, &sigmask);//抵挡signal较为方便
        	if(ret < 0)//说明是出错
        	{
            		if(ret == EINTR)//如果说是中断错误,认为是可以接受的错误,select继续运行
            		{
                		continue;
            		}
            		else//如果是错误,并且不是EINTR错误,认为是真正的错误,循环退出
            		{
            			perror("select");
                		break;
            		}
        	}    

        	if(FD_ISSET(0, &readfds))
        	{
            		//memset(buf.text, 0, sizeof(buf.text));
            		bzero(&send, sizeof(send));
            		send.type = TYPE_GROUPMSG;
            		strcpy(send.usrinfo.name, usr_name);
            		printf("\033[32m");//字色 30~39,设置绿色为发信   		
           		fgets(send.text, sizeof(send.text), stdin);
            		printf("\033[0m");
            		//printf("type is %d, name is %s, msg is %s\n", send.type, send.usrinfo.name, send.text);
            		write(sockfd, &send, sizeof(send));
            		if(strncmp(send.text, "quit", 4) == 0)
            		{
                		printf("正在退出群聊...\n");
                		break;
            		}
        	}

        	if(FD_ISSET(sockfd, &readfds))
        	{           		  
        		printf("\033[33m");//设置黄色为收信
            		bzero(&recv, sizeof(recv));
            		read(sockfd, &recv, sizeof(recv)); 
            		if(ret <= 0)
            		{
            			printf("\033[0m");
            			break;
            		}
			//printf("type is %d, name is %s, msg is %s\n", recv.type, recv.usrinfo.name, recv.text);			
            		if(strncmp(recv.text, "quit", 4) == 0)
            		{
                		printf("%s 已退出群聊聊天室...\n", recv.usrinfo.name);
            		}
            		printf("%s:%s\n", recv.usrinfo.name, recv.text);
            		printf("\033[0m");
        	}
    	}  

    	return;
}


void usr_exit()
{
	msg_t buf;
	buf.type = TYPE_EXIT;	
	write(sockfd, &buf, sizeof(buf));
}

void usr_offline()
{
	msg_t buf;
	buf.type = TYPE_LOGOUT;
	strcpy(buf.usrinfo.name, usr_name);	
	write(sockfd, &buf, sizeof(buf));
}


void usr_register()
{	
	msg_t buf;
	bzero(&buf, sizeof(buf));
	buf.type = TYPE_REGISTER;
	printf("创建用户名:");
	scanf("%s", buf.usrinfo.name);
	getchar();
	printf("创建密码:");
	scanf("%s", buf.usrinfo.passwd);
	getchar();
	write(sockfd, &buf, sizeof(buf));
	
	read(sockfd, &buf, sizeof(buf));
	printf("%s\n", buf.text);
}

//static int times = 0;
int usr_login()
{
	msg_t buf;
	bzero(&buf, sizeof(buf));
	buf.type = TYPE_LOGIN;
	printf("用户名:");
	scanf("%s", usr_name);
	getchar();
	strcpy(buf.usrinfo.name, usr_name);
	printf("密码:");
	printf("\033[8m");
	scanf("%s", buf.usrinfo.passwd);
	getchar();
	printf("\033[0m");
	write(sockfd, &buf, sizeof(buf));//send to server
	
	read(sockfd, &buf, sizeof(buf));//recv from server
	printf("%s\n", buf.text);
	/*if(buf.type == TYPE_REPEAT)
	{
		return TYPE_REPEAT;
	}
	else if(buf.type == TYPE_ERR)
	{
		return TYPE_ERR;
	}*/
	if (buf.type == TYPE_OK)
	{
		return TYPE_OK;
	}
	else
	{
		printf("\ntry again?\tY/N");
		char choice;
		scanf("%c", &choice);
		getchar();
		if(choice == 'y' || choice == 'Y')
		{
			usr_login();
		}
		else
		{
			return TYPE_ERR;
		}
		return TYPE_ERR;
	}
}


void cmd()
{
	msg_t buf;
	bzero(&buf, sizeof(buf));
	buf.type = TYPE_CMD;
	printf("cmd:");
	fgets(buf.text, sizeof(buf.text), stdin);
	write(sockfd, &buf, sizeof(buf));
	bzero(&buf, sizeof(buf));
	
	printf("\033[36m");
	read(sockfd, buf.text, sizeof(buf.text));
	printf("\n%s\n", buf.text);
	printf("\033[0m");		
}


void usr_chat()
{
	int flag = 1;
	while(flag)
	{
		sleep(1);
		system("clear");
		printf("\t\t\t********Let's Chat********\n");
		//printf("1.私聊\n");
		printf("\t\t\t##         1.群聊      ##\n");
		printf("\t\t\t##         2.离线      ##\n");
		printf("\t\t\t**************************\n");
		printf("请选择：");
		int choice;
		scanf("%d", &choice);
		getchar();
	
		switch(choice)
		{
			//case 1:
			//	single_chat();
			//	break;
			
			case 1:
				group_chat();
				break;
			
			case 2:
				usr_offline();
				flag = 0;
				break;
			
			default:
				break;			
		}
	}
	
}



