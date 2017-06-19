#include "common.h"


int sockfd;
sigset_t sigmask;

extern void usr_register();
extern int usr_login();
extern void cmd();
extern void usr_chat();
extern void usr_exit();

void heart_beat(int signum)
{
	if(signum == SIGALRM)
	{
		msg_t buf;
		buf.type = TYPE_HEART;
		write(sockfd, &buf, sizeof(buf));
		alarm(3);
	}
}


int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("请至少附加一个端口号!\n");
		printf("建议 IP + 端口号\n");
		exit(1);
	}
	//pselect能够避免下面的状况
	if(signal(SIGALRM, heart_beat) == SIG_ERR)
	{
		perror("signal SIGALRM error");
	}
	
	if(sigemptyset(&sigmask) < 0)
	{
		perror("sigemptyset");
	}
	if(sigaddset(&sigmask, SIGALRM) < 0)
	{
		perror("sigaddset");
	}

/*      这种方法会彻底屏蔽SIGALRM信号，尽管select不会被中断但是心跳包无法发送了
	
	if(sigemptyset(&sigmask) < 0)
	{
		perror("sigemptyset");
	}
	if(sigaddset(&sigmask, SIGALRM) < 0)
	{
		perror("sigaddset");
	}
*******	sigprocmask(SIG_BLOCK, &sigmask, NULL);
	
*/

/*	可使被中断的系统调用恢复，但是这种方法并不适用于所有系统调用，目前情况看来select不行
	struct sigaction action; 
	action.sa_handler = heart_beat; 
	sigemptyset(&action.sa_mask); 
	action.sa_flags = 0; 
	//设置SA_RESTART属性 *
	action.sa_flags |= SA_RESTART; 
	if(sigaction(SIGALRM, &action, NULL) < 0)
		perror("sigaction");
*/	
	
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5678);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr.s_addr);
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("socket");
		exit(1);
	}
	int ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if(ret < 0)
	{
		perror("connect");
		exit(1);
	}
	
	int flag = 1;
	//int times = 0;

	while(flag)
	{
		sleep(1);
		system("clear");
		printf("\t\t\t*********我的远程终端**********\n");
		printf("\t\t\t##         1.注册           ##\n");
		printf("\t\t\t##         2.登录           ##\n");
		printf("\t\t\t##         3.shell命令      ##\n");
		printf("\t\t\t##         4.退出           ##\n");
		printf("\t\t\t*******************************\n");
		int choice;
		scanf("%d", &choice);
		getchar();
		switch(choice)
		{
			case 1://register
				usr_register();
				break;
			case 2://login
				/*times++;
				if(times > 3)
				{
					printf("try more than 3 times!\nexiting...\n");
					flag = 0;
					break;
				}*/
				ret = usr_login();
				if(ret == TYPE_OK)
				{
					alarm(3);//heart beating...
					printf("转入聊天室....\n");
					usr_chat();
				}
				break;
			case 3://cmd
				cmd();
				break;
			case 4://exit
				flag = 0;
				usr_exit();
				break;
			default:
				break;
		}

	}
		
	close(sockfd);
	return 0;
}


