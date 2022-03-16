#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctime>
#include <vector>
#include <errno.h>
#include <chrono>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define PORT 3677
#define BUFFER_LEN 1024
#define MAX_CONNECTION 128
#define ADDRESS_LEN 50

pthread_t threadID[MAX_CONNECTION]; //store every thread
int ID = 0; //thread id threadID[ID]
char* addrList[MAX_CONNECTION];
int portList[MAX_CONNECTION];
int fdList[MAX_CONNECTION];
int isConnect[MAX_CONNECTION]; //the address and port is connected or not

void *handlerRequest(void *arg){
    int sockfd = *((int*)arg);
    int maxfd;
    struct timeval tv;
    fd_set fds;
    int returnValue;
    char dataSend[BUFFER_LEN];
    char dataReceive[BUFFER_LEN];
    char dataTemp[BUFFER_LEN]; //temp data
    int sendLen, receiveLen;
    const char* reply = "Reply Message From Server!\n";
    char serverTime[BUFFER_LEN];
    char serverName[BUFFER_LEN];
    char sendMessage[BUFFER_LEN];
    int currentID = ID - 1;
    int sendMessageID;
    fdList[ID] = sockfd;
    //reply to client
    if( send(sockfd, reply, strlen(reply), 0) != -1){
        cout << "Reply Message Sent Successfully!" << endl;
    }else{
        perror("Error : Message Sent To Client Failed!\n");
        exit(errno);
    }

    while(true){
        //setup fds
        FD_ZERO(&fds);  //init fds
        FD_SET(0, &fds);//add 0 to set
        maxfd = 0;      
        FD_SET(sockfd, &fds);   //add sockfd to fdset
        if(sockfd > maxfd){
            maxfd = sockfd; //swap
        }
        tv.tv_sec = 6; //seconds
        tv.tv_usec = 0;
        returnValue = select(maxfd + 1, &fds, NULL, NULL, &tv);
        if(returnValue == -1){
            perror("Error : Select Failed, Quit Client!\n");
            break;
        }
        else if(returnValue == 0){
            continue;
        }
        else{
            //server send message
            if(FD_ISSET(0, &fds)){
                bzero(dataSend, BUFFER_LEN);        //init
                fgets(dataSend, BUFFER_LEN, stdin); //input
                if( !strncasecmp(dataSend, "quit", 4) || !strncasecmp(dataSend, "exit", 4) ){
                    cout << "Server Required To Quit!" << endl;
                    break;
                }
                sendLen = send(sockfd, dataSend, strlen(dataSend), 0);
                if(sendLen > 0){
                    cout << "Data Send Successfully!" << endl;
                }else { // < 0 / -1
                     perror("Error : Server Didn't Send Your Message!\n");
                     break;
                }
            }

            //receive message from client
            if(FD_ISSET(sockfd, &fds)){
                bzero(dataReceive, BUFFER_LEN);
                receiveLen = recv(sockfd, dataReceive, BUFFER_LEN, 0);
                if(receiveLen > 0){
                    // send message to client format, got four type 
                    // TIME%s/NAME%s/LIST%s/SENDID: %d, Address: %s, Port: %d, Sent A Message To You:#%s#
                    cout << "Data Packet Receive From Client : " << dataReceive << endl;
                    //if is time request
                    if( !strncmp(dataReceive, "time", 4) ){
                        //time today
                        using chrono::system_clock;
                        system_clock::time_point timeToday = system_clock::now();
                        time_t t;
                        t = system_clock::to_time_t(timeToday);
                        char time[BUFFER_LEN];
    					strcpy(time,ctime(&t));
    					//cout << time << endl;
                        sprintf(serverTime, "TIME%s",time);
                        if( send(sockfd, serverTime, sizeof(time), 0) > 0 ){
                            cout << "Time Data Has Been Sent To CLient " << currentID + 1 << "!"<< endl;
                        }else{
                            //send fail
                            perror("Error : Server Send Time Data Message Failed!\n");
                            exit(errno);
                        }
                    }
                    else if( !strncmp(dataReceive, "name", 4) ) { //name
                        char tempName[100];
                        bzero(tempName, 100);
                        gethostname(tempName, 100);
                        sprintf(serverName, "NAME%s", tempName);
                        //cout << tempName;
                        if( send(sockfd, serverName, strlen(serverName), 0) > 0 ){
                            cout << "Server Name Has Been Sent To CLient " << currentID + 1 << "!"<< endl;
                        }else{
                            //send fail
                            perror("Error : Server Send Server Name Message Failed!\n");
                            exit(errno);
                        }
                    }
                    else if( !strncmp(dataReceive, "list", 4) ) { //list
                        //traverse the client list
                        for(int i = 0 ; i < ID ; i++){
                            bzero(dataSend, BUFFER_LEN);
                            if(isConnect[i] == 1){
                                //check if this client id is connect or not
                                sprintf(dataSend, "LISTID : %d, Address : %s, Port : %d\n",i+1, addrList[i], portList[i]);
                                if(send(sockfd, dataSend, strlen(dataSend), 0) > 0){
                                    //send success
                                    cout << "[CLient ID : " << i+1 << "] Data Message Has Been Sent To Client!" << endl;
                                    sleep(0.5);
                                }else{
                                    //send fail
                                    perror("Error : Server Send Client Data Message Failed!");
                                    exit(errno);
                                }
                            }else{
                                cout << "Client " << i+1 << " Is Not Connected" << endl;
                            }
                        }
                    }
                    else if(!strncmp(dataReceive, "send", 4) ){ //send message to server
                        //get client id
                        bzero(dataTemp, BUFFER_LEN);
                        int j = 0,i = 0;
                        for(i = 5 ; i < BUFFER_LEN ; i++){
                            //data receive from client string is : send#id#msg\n
                            if(dataReceive[i] != '#'){
                                dataTemp[j] = dataReceive[i];
                                j++;
                            }else{ // == '#'
                                break;
                            }
                        }
                        sendMessageID = atoi(dataTemp);
                        j = 0;
                        bzero(sendMessage, BUFFER_LEN);
                        for(i++ ; i < BUFFER_LEN ; i++){
                            if(dataReceive[i] != '\n'){
                                sendMessage[j] = dataReceive[i];
                                j++;
                            }else{ // == '\n'
                                break;
                            }
                        }
                        bzero(dataSend, BUFFER_LEN);
                        sprintf(dataSend, "SENDID: %d, Address: %s, Port: %d Sent A Message To You: %s"
                                , currentID+1, addrList[currentID], portList[currentID], sendMessage);
                        if( send(fdList[sendMessageID], dataSend, strlen(dataSend), 0) > 0 ){
                            cout << "Message From Client : " << currentID+1 << " Has Been Sent To Client : " << sendMessageID << endl;
                        	cout << "[Message] " << sendMessage << endl;
						}else{
                            perror("Error : Server Resend Message To Client Failed");
                            exit(errno);
                        }
                    }
                }
//                else{
//                    perror("Error : Server Receive Message Failed!\n");
//                }
            }
        }
    }
    //clear current pthread
    cout << "Terminating Connection On Current Client!" << endl;
    isConnect[ID] = 0;  //this client disconnect
    close(sockfd);      //close fd
    pthread_exit(NULL); //exit current pthread
    
}

class Server{
    public:
        Server();
        ~Server();
        int initial(pthread_t tid);
        int serverBind();
        int serverListen();
        int serverHandler();

    private:
        struct sockaddr_in serverAddress;
        struct sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        pthread_t tID;
        int serverSocket = -1;
        int clientSocket = -1;
};

Server::Server(){}
Server::~Server(){}

int Server::initial(pthread_t tid){
    tID = tid;
    bzero(isConnect, MAX_CONNECTION); //clear client info
    //create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0); //IPV4
    if(serverSocket != -1) {
        cout << "[Socket Created] Sucess!" << endl;
    }
    else{ // == -1
        perror("Error : Create Socket Fail!\n");
        exit(errno);
    }
}

int Server::serverBind(){
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(PORT);
    if( bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(struct sockaddr)) != -1 ){
        //bind success
        cout << "[Bind Socket] Success!" << endl;
    }else{ // == -1
        perror("Error : Bind Failed!\n");
        exit(errno);
    }
}

int Server::serverListen(){
    if( listen(serverSocket, MAX_CONNECTION) != -1 ){
        //bind success
        cout << "[Listen Socket] Success!" << endl;
    }else{ // == -1
        perror("Error : Listen Failed!\n");
        exit(errno);
    }
}

int Server::serverHandler(){
    while(true){
        //accept sock
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressSize);
        if(clientSocket == -1){
            //accept fail
            perror("Error : Accept Failed!\n");
            continue;
        }
        else{
            //accept success
            cout << "[Connection Established] Client Address : " << inet_ntoa(clientAddress.sin_addr)
                 << " PORT : " << ntohs(clientAddress.sin_port) << endl;
            addrList[ID] = (char*)malloc(sizeof(char) * ADDRESS_LEN); // request size
            strcpy(addrList[ID], inet_ntoa(clientAddress.sin_addr));
            portList[ID] = ntohs(clientAddress.sin_port);
            isConnect[ID] = 1;
            //create new thread
            if(pthread_create(&threadID[ID], NULL, (&handlerRequest), (void *)(&clientSocket)) == -1 ){
                //thread create fail
                ID--;
                perror("Error : Create New Thread Failed!\n");
                break;
            }
            //else ID++, next thread
            ID++;
      }
    }
    //shutdown socket
    if(shutdown(serverSocket, SHUT_WR) == -1){
        perror("Error : Shut Down Failed!\n");
    }else{
        cout << "Server Shut Down!" << endl;
    }
    return 0;
}

int main(){
    pthread_t tid;
    Server *server = new Server();
    server->initial(tid);
    server->serverBind();
    server->serverListen();
    server->serverHandler();
    return 0;
}

