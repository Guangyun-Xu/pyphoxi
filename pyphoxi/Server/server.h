#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <signal.h>
#include <iomanip>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

typedef void *(*THREADFUNCPTR)(void *);

class Server {

    public:
        Server(int port);
        void *listener_thread();
        void init_listener_thread();
        void update_buffer(const unsigned char *data, int offset, unsigned long numbytes);

    private:
        int init_sock, conn_sock;
        char *send_buffer;
        int buffer_size = 1024;
        char receive_buffer[1024];
        struct sockaddr_in serv_addr;
        struct sockaddr_storage serv_storage;
        socklen_t addr_size;
        pthread_mutex_t buffer_access_mutex;
        pthread_t listener_thread_id;
        unsigned long frame_size;
};

#endif
