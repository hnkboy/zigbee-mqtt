#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/epoll.h>

#include "infra.h"
#include "serijson.h"
/* 最大缓存区大小 */
#define MAX_BUFFER_SIZE 8096


#define NUM_EVENTS 10

int main(int argn, char **argc)
{
	const int on = 1;

	int tcp_sock, udp_sock, from_len, ready_tcp, ready_udp, ready, contact, len, epfd, fd, i;
    int client_sock;
	int timeout_msec = 500;
	char buf[30];
	struct sockaddr_in s_addr, clnt_addr, new_s_addr;
	struct epoll_event ev, events[NUM_EVENTS];
    ushort server_port, client_port;

    if (argn <= 2)
    {
        printf("arg: recvport sendport");
        exit (1);
    }
    server_port = (ushort)atoi(argc[1]);
    client_port = (ushort)atoi(argc[2]);

    printf("zigbee series to json, port:%d client:%d\n",server_port, client_port);

	epfd = epoll_create (NUM_EVENTS); /*epoll descriptor init*/
/*
 * Fabricate socket and set socket options.
 */
	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /*INADDR_LOOPBACK*/
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
	clnt_addr.sin_addr.s_addr =  htonl (INADDR_LOOPBACK); //inet_addr("127.0.0.1");
	clnt_addr.sin_port = htons (client_port);

/*
 * client TCP-socket part.
 */
	printf ("Connecting to server...\n");
	connect (client_sock, (struct sockaddr *)&clnt_addr, sizeof(clnt_addr));

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = client_sock;
	ready_tcp = epoll_ctl (epfd, EPOLL_CTL_ADD, client_sock, &ev);
	if (ready_tcp < 0)
	{
		perror ("Epoll_ctl TCP error!\n");
		exit (1);
	}
    send (client_sock, "Hi! I'm TCP client!\n", 21, 0);
    printf ("Connecting success...\n");
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
            else if (events[i].data.fd == client_sock)
            {
                char buffer[MAX_BUFFER_SIZE];
                int ret;

                memset(buffer, 0, MAX_BUFFER_SIZE);
                ret = recv( events[i].data.fd, buffer, MAX_BUFFER_SIZE, 0);
                if (ret > 0)
                {
                    printf("收到消息:%s, 共%d个字节\n", buffer, ret);
                    printf("buf: %s\n", buffer);
                }
                else
                {
                    if (ret == 0)
                        printf("服务器主动关闭连接！！！\n");
                    close(events[i].data.fd);
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                char buffer[MAX_BUFFER_SIZE];
                char outbuffer[MAX_BUFFER_SIZE];
                int ret;

                memset(buffer, 0, MAX_BUFFER_SIZE);
                ret = recv( events[i].data.fd, buffer, MAX_BUFFER_SIZE, 0);
                if (ret > 0)
                {

                    (void)seri_to_json(buffer, ret, outbuffer);
                    //send (events[i].data.fd, "It's for TCP client!\n", 22, 0);
                    printf("收到消息:%s, 共%d个字节\n", buffer, ret);
                    printf("outbuf: %s\n", outbuffer);
                    send (client_sock, outbuffer, strlen(outbuffer), 0);
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
	closesocket (tcp_sock);
	closesocket (udp_sock);
	close (epfd);
	return 1;
}

