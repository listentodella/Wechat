#include "common.h"

extern int sockfd;
extern int connfd;
extern sockinfo_t client_info;
extern info_t *head;


void heart(sockinfo_t tmp, msg_t buf)
{
	time_t t;
	time(&t);
	client_info.time = ctime(&t);
	printf("client ip %s is ALIVE! time is %s", tmp.ip, client_info.time);
}


void usr_register(sockinfo_t tmp, msg_t buf)
{
	int fd = open("./usrinfo.txt", O_CREAT | O_RDWR, 0644);
	if(fd < 0)
	{
		perror("open usrinfo.txt");
		exit(1);
	}
	
	msg_t data;
	
	while(read(fd, &(data.usrinfo), sizeof(data.usrinfo)))
	{
		if(strcmp(buf.usrinfo.name, data.usrinfo.name) == 0)
		{
			buf.type = TYPE_ERR;
			strcpy(buf.text, "用户名已存在!");
			printf("User %s register repeat...\n", buf.usrinfo.name);
			write(tmp.sockfd, &buf, sizeof(buf));
			close(fd);
			return;
		}
	}

	int ret = write(fd, &buf.usrinfo, sizeof(buf.usrinfo));
	if(ret < 0)
	{
		perror("write usrinfo");
		exit(1);
	}

	info_t *p = head;
	info_t *tmpnode = NULL;
	while(p != NULL)
	{
		tmpnode = p;
		p = p->next;
	}

	info_t *q = (info_t *)malloc(sizeof(info_t));
	if(NULL == q)
	{
		perror("malloc");
		exit(1);
	}
	strcpy(q->name, buf.usrinfo.name);
	strcpy(q->passwd, buf.usrinfo.passwd);
	q->clinfo.sockfd = -1;
	q->online = 0;
	
	if(NULL == head)
	{
		head = q;
	}
	else
	{
		tmpnode->next = q;
	}
	q->next = NULL;
			
	buf.type = TYPE_OK;
	strcpy(buf.text, "注册成功！");
	printf("User %s register successfully...\n", buf.usrinfo.name);
	write(tmp.sockfd, &buf, sizeof(buf));
		
	close(fd);
	return;
}


void usr_login(sockinfo_t tmp, msg_t buf)
{
	//int i = 0;
	printf("User %s tring to login...\n", buf.usrinfo.name);
	info_t *p = head;
	while(p != NULL)
	{
		if(strcmp(p->name, buf.usrinfo.name) == 0)
		{
			if(strcmp(p->passwd, buf.usrinfo.passwd) == 0)
			{
				if(p->online == 1)
				{
					buf.type = TYPE_REPEAT;
					strcpy(buf.text, "重复登录!");
					write(tmp.sockfd, &buf, sizeof(buf));
					printf("User %s login already...\n", buf.usrinfo.name);
					p = NULL;
					return;
				}
				else
				{
					p->online = 1;//更新为在线状态
					p->clinfo.sockfd = tmp.sockfd;					
					buf.type = TYPE_OK;
					strcpy(buf.text, "登录成功!");
					write(tmp.sockfd, &buf, sizeof(buf));
					printf("User %s login successfully...\n", buf.usrinfo.name);
					p = NULL;
					return;				
				}							
			}
			else
			{
				//i++;
				buf.type = TYPE_ERR;
				strcpy(buf.text, "密码错误!");
				write(tmp.sockfd, &buf, sizeof(buf));
				printf("User %s login failed...\n", buf.usrinfo.name);
				p = NULL;
				return;
			}
		}
		else
		{
			p = p->next;
		}
	}
	
	buf.type = TYPE_ERR;
	strcpy(buf.text, "用户名不存在!");
	write(tmp.sockfd, &buf, sizeof(buf));
	printf("User %s login failed...\n", buf.usrinfo.name);
	p = NULL;
	return;
}


void usr_logout(sockinfo_t tmp, msg_t buf)
{
	printf("User %s tring to logout...\n", buf.usrinfo.name);
	info_t *p = head;
	while(p != NULL)
	{
		if(strcmp(p->name, buf.usrinfo.name) == 0)
		{			
			p->online = 0;//离线状态
			//p->clinfo.sockfd = 0;//离线还是不清零了。。。免得close出错
			buf.type = TYPE_OK;
			strcpy(buf.text, "离线成功!");
			write(tmp.sockfd, &buf, sizeof(buf));
			printf("User %s logout successfully...\n", buf.usrinfo.name);
			p = NULL;
			return;				
		}
		p = p->next;
	}
}


void cmd(sockinfo_t tmp, msg_t buf)
{
	int tmpfdout = dup(STDOUT_FILENO);
	int ret = dup2(tmp.sockfd, STDOUT_FILENO);
	if(ret < 0)
	{
		perror("1 dup2");
		exit(1);
	}
	system(buf.text);
	ret = dup2(tmpfdout, STDOUT_FILENO);
	if(ret < 0)
	{
		perror("2 dup2");
		exit(1);
	}
	close(tmpfdout);
}


info_t *usr_linklist_create()
{
	int fd = open("./usrinfo.txt", O_CREAT | O_RDWR, 0644);
	if(fd < 0)
	{
		perror("open usrinfo.txt");
		exit(1);
	}
			
	info_t *p = NULL;
	info_t *q = NULL;
	
	info_t tmp_usrinfo;
	while(read(fd, &tmp_usrinfo, sizeof(tmp_usrinfo)))
	{
		q = (info_t *)malloc(sizeof(info_t));
		if(NULL == q)
		{
			perror("malloc");
			exit(1);
		}
		strcpy(q->name,tmp_usrinfo.name);
		strcpy(q->passwd, tmp_usrinfo.passwd);
		q->clinfo.sockfd = -1;//每个客户端对应的connfd 在链表创建时先初始化为0,每一个客户端链接后再将对应的connfd赋值？
					//还是每个用户登录之后再赋值？这个方案更合理一些
		q->online = 0;
		if(NULL == head)
		{
			head = q;
			p = q;
		}
		else
		{
			p->next = q;
			p = q;
		}
		p->next = NULL;
	}

	close(fd);
	return head;
}


void display_all()
{
	info_t *p = head;
	while(p != NULL)
	{
		printf("name:%s online:%d\n", p->name, p->online);
		p = p->next;
	}
}


void display_online()
{
	info_t *p = head;
	//while(p != NULL && p->online == 1)
	while(p != NULL)
	{
		if(p->online == 1)
			printf("name:%s online:%d\n", p->name, p->online);
		p = p->next;
	}
}


void group_chat_handler(sockinfo_t tmp, msg_t buf)
{
	info_t *p = head;
	printf("User %s join the groupchat...\n", buf.usrinfo.name);
	display_online();
	msg_t recv;
	while(1)
	{
		bzero(&recv, sizeof(recv));
		read(tmp.sockfd, &recv, sizeof(recv));
		//printf("type is %d, name is %s, msg is %s\n", recv.type, recv.usrinfo.name, recv.text);	
		
		//while((p != NULL) && (p->online == 1))//只对在线的用户转发
		if(recv.type == TYPE_HEART)
		{
			heart(tmp, buf);
			continue;
		}
		while(p != NULL)
		{
			if(strcmp(buf.usrinfo.name, p->name) == 0)
			{
				p = p->next;
				continue;
			}

			if(p->online == 1)//筛选出在线客户端
			{
				printf("send msg to %s\n", p->name);
				write(p->clinfo.sockfd, &recv, sizeof(recv));//将收到的信息转发给其他在线客户端
			}			
			p = p->next;
		}
		printf("User %s sent a group msg...\n", buf.usrinfo.name);
		p = head;//用于下一次遍历、转发
		
		if(strncmp(recv.text, "quit", 4) == 0)
		{
			printf("User %s exiting the groupchat...\n", buf.usrinfo.name);
			break;
		}
	}
	return;
}



