#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <kvstore.h>

// 线程不安全
int epfd = 0;

// 线程不安全
struct conn_item connlist[1024] = {0};

// 1为新增, 0为修改
void set_event(int fd, int event, int flag)
{
    if(flag)
    {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    }
    else
    {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
    }
}

int recv_cb(int fd);
int send_cb(int fd);

// listenfd
int accept_cb(int fd)
{
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int clientfd = accept(fd, (struct sockaddr *)&caddr, &len);

    if(clientfd < 0)
    {
        return -1;
    }

    set_event(clientfd, EPOLLIN, 1);

    connlist[clientfd].fd = clientfd;
    memset(connlist[clientfd].rbuffer, 0, BUFFER_LENGTH);
    connlist[clientfd].rlen = 0;
    memset(connlist[clientfd].wbuffer, 0, BUFFER_LENGTH);
    connlist[clientfd].wlen = 0;
    connlist[clientfd].recv_t.recv_callback = recv_cb;
    connlist[clientfd].send_callback = send_cb;

    return clientfd;
}

// clientfd
int send_cb(int fd)
{
    char *buffer = connlist[fd].wbuffer;
    int idx = connlist[fd].wlen;

    int count = send(fd, buffer, idx, 0);

    set_event(fd, EPOLLIN, 0);

    return count;
}

int recv_cb(int fd)
{
    char *buf = connlist[fd].rbuffer;

    int count = recv(fd, buf, BUFFER_LENGTH, 0);
    if (count == 0)
    {
        //std::cout << "disconnect" << std::endl;

        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);

        return -1;
    }
    connlist[fd].rlen = count;
    
    kvstore_request(&connlist[fd]);
    connlist[fd].wlen = strlen(connlist[fd].wbuffer);

    set_event(fd, EPOLLOUT, 0);

    return count;
}

int epoll_entry(void)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(2048);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (sockaddr*)&saddr, sizeof(saddr)) == -1)
    {
        perror("bind func error!");
        return -1;
    }

    listen(sockfd, 128);

    connlist[sockfd].fd = sockfd;
    connlist[sockfd].recv_t.accept_callback = accept_cb;

    // epoll多路复用
    epfd = epoll_create(1);
    set_event(sockfd, EPOLLIN, 1);
    struct epoll_event events[1024] = {0};
    while (1)
    {
        int nready = epoll_wait(epfd, events, 1024, -1);
        for (int i = 0; i < nready; ++i)
        {
            int connfd = events[i].data.fd;
            if(events[i].events & EPOLLIN)
            {
                int count = connlist[connfd].recv_t.recv_callback(connfd);

                //std::cout << "recv <--- buf:" << std::endl << connlist[connfd].rbuffer << std::endl;
            }
            else if(events[i].events & EPOLLOUT)
            {
                int count = connlist[connfd].send_callback(connfd);

                //std::cout << "send ---> buf:" << std::endl << connlist[connfd].wbuffer << std::endl;
            }
        }
    }
    return 0;
}