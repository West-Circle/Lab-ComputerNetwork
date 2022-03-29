#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <iostream>
#include <cstring>
#include <stdio.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/mman.h>
#define MAXCONN 1024

class Network {
public:
    Network();
    void Listen(int port);
    void Close();
    int Acccept();
private:
    int conn_fd;
    int listen_fd;
};

#endif
