#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/epoll.h>


#include "infra.h"
#include "serijson.h"
#include "list.h"
#include "coor.h"
/* 最大缓存区大小 */
#define MAX_BUFFER_SIZE 8096


#define NUM_EVENTS 10

int main(int argn, char **argc)
{
	const int on = 1;

	int tcp_sock, udp_sock, from_len, ready_tcp, ready_udp, ready, contact, len, epfd, fd, i;
    int client_sock;
    uint server_addr,client_addr;
	int timeout_msec = 500;
    int client_con_ret;
	char buf[30];
	struct sockaddr_in s_addr, clnt_addr, new_s_addr;
	struct epoll_event ev, events[NUM_EVENTS];
    ushort server_port, client_port;

    /*1. 打开syslog*/
    openlog("SERID", LOG_CONS | LOG_PID, LOG_LOCAL2);
    
    /*2. 初始化 coor 设备资源*/
    coorfd_init();
    if (argn <= 2)
    {
        printf("arg: recvport sendport");
        exit (1);
    }
    inet_pton(AF_INET, argc[1], (void*)&server_addr);
    server_port = (ushort)atoi(argc[2]);
    inet_pton(AF_INET, argc[3], (void*)&client_addr);
    client_port = (ushort)atoi(argc[4]);


    printf("zigbee series to json, port:%d client:%d\n",server_port, client_port);

    printf("zigbee series to json, addr:%u client addr:%u\n",server_addr, client_addr);

	epfd = epoll_create (NUM_EVENTS); /*epoll descriptor init*/
/*
 * Fabricate socket and set socket options.
 */
	s_addr.sin_family = AF_INET;
	//s_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /*INADDR_LOOPBACK*/
    s_addr.sin_addr.s_addr = server_addr;
	s_addr.sin_port = htons (server_port);
/*
 * TCP-socket part.
 */
	tcp_sock = socket (AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	setsockopt (tcp_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
	if (bind (tcp_sock, (struct sockaddr *)&s_addr, sizeof (s_addr)) < 0)
	{
		perror ("TCP bind error!\n");
		exit (1);
	}
	ev.events = EPOLLIN | EPOLLPRI | EPOLLET;
	ev.data.fd = tcp_sock;
	ready_tcp = epoll_ctl (epfd, EPOLL_CTL_ADD, tcp_sock, &ev);
	if (ready_tcp < 0)
	{
		perror ("Epoll_ctl TCP error!\n");
		exit (1);
	}
	listen(tcp_sock, 1);

/*
 * UDP-socket part.
 */
	udp_sock = socket (AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if (bind (udp_sock, (struct sockaddr *)&s_addr, sizeof (s_addr)) < 0)
	{
		perror ("UDP bind error!\n");
		exit (1);
	}
	ev.events = EPOLLIN | EPOLLPRI | EPOLLET;
	ev.data.fd = udp_sock;
	ready_udp = epoll_ctl (epfd, EPOLL_CTL_ADD, udp_sock, &ev);
	if (ready_udp < 0)
	{
		perror ("Epoll_ctl UDP error!\n");
		exit (1);
	}
/*
 * Fabricate socket and set socket options.
 */
	client_sock = socket (AF_INET, SOCK_STREAM, 0);
	clnt_addr.sin_family = AF_INET;
	//clnt_addr.sin_addr.s_addr =  htonl (INADDR_LOOPBACK); //inet_addr("127.0.0.1");
    clnt_addr.sin_addr.s_addr =  client_addr;
	clnt_addr.sin_port = htons (client_port);

/*
 * client TCP-socket part.
 */
	printf ("Connecting to server...\n");
    /*返回值只是判断是否成功，client角色只需要一个fd就可以搞定*/
	client_con_ret = connect (client_sock, (struct sockaddr *)&clnt_addr, sizeof(clnt_addr));
    if(client_con_ret == (-1))
    {
        perror ("Connect mqtt error!\n");
        exit (1);
    }
 	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = client_sock;
	ready_tcp = epoll_ctl (epfd, EPOLL_CTL_ADD, client_sock, &ev);

	if (ready_tcp < 0)
	{
		perror ("Epoll_ctl TCP error!\n");
		exit (1);
	}
    //send (client_sock, "Hi! I'm TCP client!\n", 21, 0);
    printf ("Connecting success...\n");
    syslog(LOG_INFO, "Connecting success... \n");
/*
 * Client service loop
 */
	while (1)
	{
		ready = epoll_wait (epfd, events, NUM_EVENTS, timeout_msec);
		if ( ready < 0)
		{
			printf ("Epoll_wait error!\n");
			exit (1);
		}
		for (i = 0; i < ready; i++)
		{
			if (events[i].data.fd == tcp_sock) /*tcp-client service*/
			{
				len = sizeof (s_addr);
				contact = accept (tcp_sock,
					(struct sockaddr *)&s_addr, &len);
				if(contact == (-1))
				{
					perror ("Connect TCP error!\n");
					exit (1);
				}
				ev.data.fd = contact;
				ev.events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd , EPOLL_CTL_ADD , contact , &ev);
			}
            /*保留，暂时不用udp*/
			else if (events[i].data.fd == udp_sock) /*udp-client service*/
			{
				fd = events[2].data.fd;
				len = sizeof (s_addr);
				while (2)
				{
					from_len = recvfrom (udp_sock, buf,
					21, 0, (struct sockaddr *)&s_addr,
					&len);
					if(from_len > 0)
					{
						write (1, buf, from_len);
						break;
					}
				}
				sendto(udp_sock, "It's for UDP client!\n", 22,
				0, (struct sockaddr *)&s_addr, sizeof(s_addr));
			}
            /*作为客户端连接mqtt*/
            else if (events[i].data.fd == client_sock)
            {
                char buffer[MAX_BUFFER_SIZE];
                char seribuffer[MAX_BUFFER_SIZE];
                int serilen;
                ushort coorid;
                int ret;
                int json_ret;
                int coor_fd = -1;
                memset(buffer, 0, MAX_BUFFER_SIZE);
                memset(seribuffer, 0, MAX_BUFFER_SIZE);
                ret = recv( events[i].data.fd, buffer, MAX_BUFFER_SIZE, 0);
                if (ret > 0)
                {
                    printf("收到消息:%s, 共%d个字节\n", buffer, ret);
                    printf("buf: %s\n", buffer);
                    json_ret = json_msgproc(buffer, ret, seribuffer, &serilen, &coorid);
                    printf("ret: %d\n", json_ret);
                    if(ERROR_SUCCESS == json_ret)
                    {
                        printf("buf: %s\n", seribuffer);
                        /*根据coorid地址找到对应的fd分发*/
                        coor_fd = coor_findfd(coorid);
                        printf("coor_fd: %d\ncoorid:%d\n", coor_fd, coorid);
                        if(coor_fd > -1)
                        {
                            send (coor_fd, seribuffer, serilen, 0);
                        }
                    }
                }
                else
                {
                    if (ret == 0)
                        printf("服务器主动关闭连接！！！\n");
                    close(events[i].data.fd);
                    exit(1);
                }
            }
            /*接收 zigbee客户端 报文*/
            else if (events[i].events & EPOLLIN)
            {
                char buffer[MAX_BUFFER_SIZE];
                char outbuffer[MAX_BUFFER_SIZE];
                int ret;
                int err;
                ushort coor_id = 0;

                memset(buffer, 0, MAX_BUFFER_SIZE);
                ret = recv( events[i].data.fd, buffer, MAX_BUFFER_SIZE, 0);
                if (ret > 0)
                {
                    err = seri_msgproc(buffer, ret, outbuffer, &coor_id);

                    //send (events[i].data.fd, "It's for TCP client!\n", 22, 0);
                    printf("收到消息:%s, 共%d个字节\n", buffer, ret);

                    /*
                    * 发送到zigee-mqtt服务器*
                    */
                    if (ERROR_FAILED != err)
                    {
                        printf("outbuf: %s\nfd:%d\n,coor_id:%d\n", outbuffer, events[i].data.fd, coor_id);
                        coor_add(coor_id, events[i].data.fd);

                        send (client_sock, outbuffer, strlen(outbuffer), 0);
                    }
                    outbuffer[0] = '\0';

                }
                else
                {
                    if (ret == 0)
                        printf("客户端主动关闭连接！！！\n");
                    close(events[i].data.fd);
                }
            }

		}
	}
	shutdown (tcp_sock, SHUT_RDWR);
	shutdown (udp_sock, SHUT_RDWR);
    shutdown (client_sock, SHUT_RDWR);
    /*SHUT_RD：断开输入流。套接字无法接收数据（即使输入缓冲区收到数据也被抹去），无法调用输入相关函数。
      SHUT_WR：断开输出流。套接字无法发送数据，但如果输出缓冲区中还有未传输的数据，则将传递到目标主机。
      SHUT_RDWR：同时断开 I/O 流。相当于分两次调用 shutdown()，其中一次以 SHUT_RD 为参数，另一次以 SHUT_WR 为参数。*/
	close (epfd);

    closelog();
	return 1;
}

