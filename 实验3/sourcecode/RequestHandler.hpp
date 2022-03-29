#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <iostream>
#include <cstring>
#include <stdio.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "csapp.h"
#define MAXBUFFER 8192
#define MAXLINE 8192

class RequestHandler {
public:
    RequestHandler(int connfd);
    void Begin();

private:
    int connfd;
};

#endif
