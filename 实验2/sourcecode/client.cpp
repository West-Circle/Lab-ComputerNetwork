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
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


using namespace std;

enum{CONNECTED, DISCONNECTED, CONNECTIONFAIL};

#define BUFFER_LEN 1024
char requestBuffer[BUFFER_LEN];
bool showMenu = false;

void *Request(void *arg){
    int sockfd = (int)(*(int*)arg);
    char serverResponse[BUFFER_LEN];
    memset(serverResponse, 0, BUFFER_LEN);
    int packetType = -1;
    while(true){
        packetType = -1;
        memset(serverResponse, 0, BUFFER_LEN);
        int res = recv(sockfd, &serverResponse, sizeof(serverResponse), 0);
        if(res > 0){
            string packet = "Connection Packet";
            if(!strncmp(serverResponse, "TIME", 4)){
                packetType = 0;//time packet type
                packet = "Time Packet";
            }
            else if(!strncmp(serverResponse, "NAME", 4)){
                packetType = 1;
                packet = "Name Packet";
            }
            else if(!strncmp(serverResponse, "LIST", 4)){
                packetType = 2;
                packet = "Client List Packet";
            }
            else if(!strncmp(serverResponse, "SEND", 4)){
                packetType = 3;
                packet = "Send Message Packet";
            }
            cout << "[Packet Received] The Packet Is " << packet << endl;
            cout << "[Received Message] : " << ( (packetType==-1) ? serverResponse : (serverResponse+4) )<< endl;
        }
        else if(res < 0){
            //receive fail
            pthread_exit(NULL);
        }
        else {// == 0
            cout << "Error : Socket Closed! Exiting Current Thread!" << endl;
            pthread_exit(NULL);
        }
    }
    close(sockfd);      //close fd
    pthread_exit(NULL); //exit current pthread
}

class Client{
    public:
        Client();
        ~Client();
        //address and port
        string getAdddress();
        void setAddress(string address);
        int getPort();
        void setPort(string port);
        //function of menu
        int initial(pthread_t tid);
        int connectServer();
        int closeConnect();
        int reqServerTime();
        int reqServerName();
        int reqClientList();
        int sendMessage();
        //menu
        void Menu();
        //execute operation
        void execOperation(int op);

    private:
        string ipAddress;
        int port;
        int sock = -1;//sockfd
        int status;
        struct sockaddr_in serverAddress;
        pthread_t tID;
        int t_id;
};

Client::Client(){}
Client::~Client(){}

string Client::getAdddress(){
    return ipAddress;
}

void Client::setAddress(string address){
    ipAddress = address;
}

int Client::getPort(){
    return port;
}

void Client::setPort(string port){
    this->port = stoi(port); //string to integer
}

int Client::initial(pthread_t tid){
    status = DISCONNECTED;
    tID = tid;
    sock = socket(AF_INET, SOCK_STREAM, 0); //socket
    return 0;
}

void Client::Menu(){
    showMenu = false;
    cout << "Please Enter Number(From 1 - 7) To Select Your Option :" << endl;
    cout << "+------------------------------------+" << endl;
    cout << "|                Menu                |" << endl;
    cout << "+------------------------------------+" << endl;
    cout << "| 1. Connect Server                  |" << endl;
    cout << "| 2. Close Connection                |" << endl;
    if(status == CONNECTED){
    	cout << "| 3. Request Server Time             |" << endl;
	    cout << "| 4. Request Server Name             |" << endl;
	    cout << "| 5. Request Client List             |" << endl;
	    cout << "| 6. Send Message To Other Client    |" << endl;
	}
	cout << "| 7. Exit Client                     |" << endl;
	cout << "+------------------------------------+" << endl;
    int op;
    cin >> op;
    execOperation(op);
}

void Client::execOperation(int op){
    switch(op){
        case 1: 
            sock = connectServer(); 
            break;
        case 2:
            if(sock != -1) closeConnect();
            break;
        case 3:
            if(status == CONNECTED) reqServerTime();
            break;
        case 4:
            if(status == CONNECTED) reqServerName();
            break;
        case 5:
            if(status == CONNECTED) reqClientList();
            break;
        case 6:
            if(status == CONNECTED) sendMessage();
            break;
        case 7:
            if(sock == -1) exit(0);
            else{
                closeConnect();
                exit(0);
            }
            break;
        deafult: break;
    }
    sleep(1);
    
}

int Client::connectServer(){
    cout << "Enter Server IP : " << endl;
    cin >> ipAddress;
    cout << "Enter Server Port : "<< endl;
    cin >> port;
    memset(&serverAddress, 0, sizeof(serverAddress));
    struct hostent *server;
    server = gethostbyname(ipAddress.c_str());
    if(server == NULL) {
        perror("Error : No Host!\n");
        exit(0);
    }
    //struct sockaddr_in serveraddr
    serverAddress.sin_family = AF_INET;
    //serverAddress.sin_addr.s_addr = inet_addr(ipAddress.c_str());
    serverAddress.sin_port = htons(port);
    inet_aton(ipAddress.c_str(), (struct in_addr *)&serverAddress.sin_addr.s_addr);
    //connect server
    int res = connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if(res == 0) {
        status = CONNECTED; //res == 0 connected
        cout << "Connect Server Success!" << endl;
    }
    //create new thread
    t_id = pthread_create(&tID, NULL, &Request, &sock);
    return sock;
}

int Client::closeConnect(){
    int s = send(sock, "quit", sizeof("quit"), 0);
    int result = close(sock);
    sock = -1;
    status = DISCONNECTED;
    if(result == 0){
    	cout << "CLose Connection To Server!" << endl;
	}
    else cout << "Close Connection Failed!" << endl;
    return result; //result == 0 success, -1 fail
} 

int Client::reqServerTime(){
    sprintf(requestBuffer, "time");
    int res = send(sock, requestBuffer, strlen(requestBuffer), 0);
    if(res == -1) {
        perror("Request Server Time Failed!\n");
        return -1;
    }else{
        cout << "Request Server Time Success!" << endl;
        return 0;
    }
}

int Client::reqServerName(){
    sprintf(requestBuffer, "name");
    int res = send(sock, requestBuffer, strlen(requestBuffer), 0);
    if(res == -1) {
        perror("Request Server Name Failed!\n");
        return -1;
    }else{
        cout << "Request Server Name Success!" << endl;    
        return 0;
    }
}

int Client::reqClientList(){
    sprintf(requestBuffer, "list");
    int res = send(sock, requestBuffer, strlen(requestBuffer), 0);
    if(res == -1) {
        perror("Request Client List Failed!\n");
        return -1;
    }else{
        cout << "Request Client List Success!" << endl;    
        return 0;
    }
}

int Client::sendMessage(){
    char msg[1014];
    char sendMsg[1024];
	int clientID;
    cout << "Please Enter The ID Of Client Which You Want To Send Message." << endl;
    cin >> clientID;
    printf("Please Enter The Message That You Want To Send.\n" );
    cin >> ws; //avoid program ignore cin.getline
    cin.getline(msg, sizeof(msg));
	sprintf(sendMsg, "send#%d#", clientID);
    strcat(sendMsg, msg); // send#ID#message...
    int res = send(sock, sendMsg, sizeof(sendMsg), 0);
    if(res == -1) {
        perror("Send Message To Server Failed!\n");
        return 1;
    }else{
        cout << "[Message] : #"<< msg << "# Send To Client " << clientID << " Success!" << endl << endl;    
        return 0;
    }
}

int main(int argc, char** argv){
    pthread_t tid;
    Client *client = new Client();
    client->initial(tid);
    showMenu = true;
    while(true){
        if(showMenu) client->Menu();
    	showMenu = true;
	}
    return 0;
}

