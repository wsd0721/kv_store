

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/time.h>
#include <bits/getopt_core.h>

#define MAX_MSG_LENGTH 512
#define TIME_SUB_MS(tv1, tv2) ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)

int send_msg(int connfd, char *msg, int length)
{
	int res = send(connfd, msg, length, 0);
	if(res < 0)
	{
		perror("send");
		exit(1);
	}
	return res;
}

int recv_msg(int connfd, char *msg, int length)
{
	int res = recv(connfd, msg, length, 0);
	if(res < 0)
	{
		perror("recv");
		exit(1);
	}
	return res;
}

void equals(char *pattern, char *result, char *casename)
{
	if(strcmp(pattern, result) == 0)
	{
		//printf("==> PASS --> %s\n", casename);
	}
	else
	{
		//printf("==> FAILED --> %s, '%s' != '%s'\n", casename, pattern, result);
	}
}

void test_case(int connfd, char *msg, char *pattern, char *casename)
{	
	if(!msg||!pattern||!casename)
	{
		return;
	}

	send_msg(connfd, msg, strlen(msg));

	char result[MAX_MSG_LENGTH] = {0};
	recv_msg(connfd, result, MAX_MSG_LENGTH);

	equals(pattern, result, casename);
}

void array_testcase(int connfd)
{
	test_case(connfd, "SET Name King", "SUCCESS", "SETCase");
	test_case(connfd, "GET Name", "King", "GETCase");
	test_case(connfd, "MOD Name Darren", "SUCCESS", "MODCase");
	test_case(connfd, "GET Name King", "Darren", "GETCase");
	test_case(connfd, "DEL Name King", "SUCCESS", "DELCase");
	test_case(connfd, "GET Name", "NO EXIST", "GETCase");
}

void array_testcase_10w(int connfd)
{
	int count = 10000;
	for (int i = 0; i < count; ++i)
	{
		array_testcase(connfd);
	}
}

int connect_tcpserver(const char *ip, unsigned short port) {

	int connfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in tcpserver_addr;
	memset(&tcpserver_addr, 0, sizeof(struct sockaddr_in));

	tcpserver_addr.sin_family = AF_INET;
	tcpserver_addr.sin_addr.s_addr = inet_addr(ip);
	tcpserver_addr.sin_port = htons(port);

	int ret = connect(connfd, (struct sockaddr*)&tcpserver_addr, sizeof(struct sockaddr_in));
	if (ret) {
		perror("connect");
		return -1;
	}

	return connfd;
}


// array: 0x01, rbtree: 0x02, hash: 0x04, skiptable: 0x08

// ./test_qps_tcpclient -s 127.0.0.1 -p 2048 -m 15
int main(int argc, char *argv[]) {

	int ret = 0;

	char ip[16] = {0};
	int port = 0;
	int mode = 1;

	int opt;
	while ((opt = getopt(argc, argv, "s:p:m:?")) != -1) {

		switch (opt) {

			case 's':
				strcpy(ip, optarg);
				break;

			case 'p':

				port = atoi(optarg);
				break;

			case 'm':
				mode = atoi(optarg);
				break;

			default:
				return -1;
		}
	}

	int connfd = connect_tcpserver(ip, port);
	
	if(mode & 0x1)
	{
		struct timeval tv_begin;
		gettimeofday(&tv_begin, NULL);

		array_testcase_10w(connfd);

		struct timeval tv_end;
		gettimeofday(&tv_end, NULL);

		int time_used = TIME_SUB_MS(tv_end, tv_begin);

		printf("time_used: %d, qps: %d\n", time_used, 60000 * 1000 / time_used);
	}
}





