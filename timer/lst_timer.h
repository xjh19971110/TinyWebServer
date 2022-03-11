#ifndef LST_TIMER
#define LST_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../log/log.h"

/*
    定时器处理非活动连接，由于非活动连接占用了连接资源，严重影响服务器的性能，通过实现一个
    服务器定时器，处理这种非活跃连接，释放连接资源。利用alarm函数周期性地触发SIGALRM信号，
    该信号的信号处理函数利用管道通知主循环执行定时器链表上的定时任务
*/
class util_timer;

struct client_data{
    sockaddr_in address;
    int sockfd;
    util_timer *timer;
};

class util_timer{
    public:
        util_timer():prev(NULL),next(NULL){}
    public:
        time_t expire;
        void(* cb_func)(client_data *);
        util_timer *prev;
        util_timer *next;
};
class sort_timer_lst
{
public:
    sort_timer_lst();
    ~sort_timer_lst();

    void add_timer(util_timer *timer);
    void adjust_timer(util_timer *timer);
    void del_timer(util_timer *timer);
    void tick();

private:
    void add_timer(util_timer *timer, util_timer *lst_head);

    util_timer *head;
    util_timer *tail;
};

class Utils{
    public:
        Utils(){}
        ~Utils(){}
        void init(int timeslot);
        //对文件描述符设置非阻塞
        int setnonblocking(int fd);
        //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
        void addfd(int epollfd,int fd,bool one_shot,int TRIGMode);
        //信号处理函数
        static void sig_handler(int sig);
        //设置信号函数
        void addsig(int sig,void(handler)(int),bool restart = true);
        //定时处理任务，重新定时以不断触发SIGALRM信号
        void timer_handler();
        void show_error(int connfd,const char *info);
    public:
        static int *u_pipefd;
        sort_timer_lst m_timer_lst;
        static int u_epollfd;
        int m_TIMESLOT;
};

void cb_func(client_data *user_data);
#endif