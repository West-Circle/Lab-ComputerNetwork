#include "Network.hpp"

//static function
static int listen_port(int port);
static int network_accept(int socket, struct sockaddr *addr, socklen_t* addrlen);

//initial listen fd and connection fd
Network::Network():listen_fd(-1), conn_fd(-1)
{
}

void Network::Listen(int port){
    int result = listen_port(port);
    if(result < 0){
        perror("[Error] Listen Failed!");
    }
    else listen_fd = result;
}

void Network::Close(){
    int res;
    res = close(conn_fd);
    if(res < 0) perror("[Error] Close Failed!");
    else return;
}

int Network::Acccept(){
    //after listen then accept
    struct sockaddr_in clientAddress;
    int clientlen = sizeof(clientAddress);
    conn_fd = network_accept(listen_fd, (struct sockaddr *)&clientAddress, (socklen_t*)&clientlen);
    if(conn_fd < 0){
        perror("[Error] Accept Failed!");
        return -1;
    }
    else return conn_fd;
}

int listen_port(int port)
{
    struct sockaddr_in serverAddress;
    int listenfd;
    int optionvalue;
    //create conn socket
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }
    //set socket option SOL_SOCKET : socket lv
    if( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optionvalue,sizeof(int)) < 0){
        return -1;//if fail
    }
    //setup server address
    bzero((char*)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons((unsigned short)port);
    if(bind(listenfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        perror("[Error] Bind Failed!");
        return -1;
    }
    //listen to socket
    if(listen(listenfd, MAXCONN) < 0){
        return -1;
    }
    return listenfd;
}

int network_accept(int socket, struct sockaddr *addr, socklen_t* addrlen){
    int result = accept(socket, addr, addrlen);
    if(result < 0){
        return -1;
    }
    else return result;
}