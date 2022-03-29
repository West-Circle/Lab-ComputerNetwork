#include "RequestHandler.hpp"
using namespace std;
//static function
static void handlePost(int connfd, char* uri, rio_t * rio);
static void handleGet(int connfd, char* uri, rio_t * rio);
static void postResponse(int connfd, bool login);
int parseURI(char* uri, char* filename, char* cgi_args);
void readRequestHeader(rio_t *rp);
void getFileType(char* filename, char* filetype);
void serve_static(int connfd, char* filename, int filesize);
void serve_dynamic(int connfd, char* filename, char* cgi_args);
void clientError(int connfd, char* err, char* shortmsg, char* longmsg);
void* handleRequest(void *arg);


RequestHandler::RequestHandler(int connfd) : connfd(connfd){

}

void RequestHandler::Begin(){
    pthread_t tid;
    //create new thread then handle request
    if( pthread_create(&tid, NULL, handleRequest, (void*)(&connfd)) == -1 ){
        perror("[Error] Creating New Thread Failed!");
        return;
    }
    //thread join
    pthread_join(tid, NULL);
}

void* handleRequest(void *arg){
    char buf[MAXBUFFER], uri[MAXLINE], version[MAXLINE], method[MAXLINE];
    rio_t rio; //struct rio
    int connfd = *(int *)arg;
    rio_readinitb(&rio, connfd); //read buffer and reset buffer associate with fd
    rio_readlineb(&rio, buf, MAXLINE); //read a text line (buffered)
    sscanf(buf, "%s %s %s", method, uri, version);
    //output to see for debug
    cout << "[Request Method] " << method << endl;
    cout << "[Request URI] " << uri << endl;
    cout << "[Request Version] " << version << endl;
    if(!strcasecmp(method, "GET")){
        //cmp == 0 , equal to method GET
        handleGet(connfd, uri, &rio);
        pthread_exit(NULL);
    }
    else if(!strcasecmp(method, "POST")){
        //cmp == 0 , equal to method POST
        handlePost(connfd, uri, &rio);
        pthread_exit(NULL);
    }
    else{
        //服务器不支持请求的方法
        clientError(connfd, "501", "Not Implemented","This Method of Request is Not Accepted By This Server");
        pthread_exit(NULL);
    }
}

void handlePost(int connfd, char* uri, rio_t * rio){
    char buf[MAXBUFFER];
    bool Login = false;
    cout << "[POST URI] : " << uri << endl;
    if(strcasecmp(uri, "/html/dopost") || strcasecmp(uri,"/dopost") < 0){
        clientError(connfd, "404", "Not Found", "This URI is Not Available In This Server");
        return ;
    }
    else{ // uri complete
        rio_readlineb(rio, buf, MAXBUFFER);
        cout << "[Server] Header : " << buf << endl; // debug
        int contentLen;
        //reading header
        while(strcmp(buf, "\r\n")){
            //keep compare until \r\n only
            if(strstr(buf, "Content-Length:")){
                //get the pointer where contentlength at
                sscanf(buf+strlen("Content-Length:"), "%d", &contentLen); //content length integer give to contentLen
            }
            rio_readlineb(rio, buf, MAXBUFFER);
            cout << buf; //debug
        }
        cout << "[Server] Reading Header Complete" << endl;
        //get body
        rio_readlineb(rio, buf, contentLen+1);
        cout << "[Server] Content Body : " << buf << endl;
        char login[MAXLINE], pass[MAXLINE];
        if(strstr(buf, "login=") && strstr(buf,"pass=")){
            char *ptr = strstr(buf, "login=");
            int l=0,p=0;
            while(*ptr++ != '='); //point after '=' then stop
            while(*ptr != '&'){
                //while ptr not point to '&'
                login[l++] = *ptr++; //get the login string
            }
            login[l] = 0;
            //same function use at password
            while(*ptr++ != '=');
            while(*ptr){
                pass[p++] = *ptr++;
            }
            pass[p] = 0;
        }
        //check login and password correct or not, 
        //login ID is student id 3190300677 and password is last 4 digit number 0677
        cout << "Login ID : " << login << " , Password : " << pass << endl;
        if(!strcmp(login, "3190300677") && !strcmp(pass, "0677")){
            Login = true;
        }else Login = false;
        postResponse(connfd, Login);
    }
}

void handleGet(int connfd, char* uri, rio_t * rio){
    char buf[MAXBUFFER], version[MAXLINE],filename[MAXLINE],cgi_args[MAXLINE];
    int isStatic;
    struct stat sbuf;
    readRequestHeader(rio);
    //parse uri from get request
    isStatic = parseURI(uri, filename, cgi_args);
    cout << "[Request] GET Filename : " << filename << endl << endl;//debug use
    if(stat(filename, &sbuf) < 0){
        clientError(connfd, "404", "Not Found", "Web Server Could Not Find This File");
        return;
    }
    if(isStatic){
        //static
        if( !(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode) ){
            //is not regular file or is not owner execute permission
            clientError(connfd, "403", "Forbidden", "Web Server Could Not Read The File");
            return;
        }
        //else serve static
        serve_static(connfd, filename, sbuf.st_size);
    }
    else{
        //dynamic
        if( !(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode) ){
            clientError(connfd, "403", "Forbidden", "Web Server Could Not Run The CGI Program");
            return;
        }
        serve_dynamic(connfd, filename, cgi_args);
    }
}

void postResponse(int connfd, bool login){
    char buf[MAXBUFFER];
    char *successResponse = "<html><body>Login Success</body></html>";
    char *failResponse = "<html><body>Login Fail</body></html>";
    cout << "Login Success" << endl;
    //send header
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, (login ? strlen(successResponse) : strlen(failResponse)) );
    sprintf(buf, "%sContent-type: text/html\r\n\r\n",buf);
    //Login successful
    rio_writen(connfd, buf, strlen(buf));
    if(login) rio_writen(connfd, successResponse, strlen(successResponse));
    else rio_writen(connfd, failResponse, strlen(failResponse));
}

int parseURI(char* uri, char* filename, char* cgi_args){
    char *ptr;
    if(!strstr(uri, "cgi-bin")){
        //string cgi-bin is not inside uri (Static)
        strcpy(cgi_args, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if(uri[strlen(uri)-1] == '/'){ //if end with '/' then default home.html filename add at behind
            strcat(filename, "home.html");
        }
        return 1;
    }
    else{
        //cgi-bin is in uri (dynamic)
        ptr = index(uri, '?');
        if(ptr){
            strcpy(cgi_args, ptr+1);
            *ptr = '\0';
        }
        else{
            strcpy(cgi_args, "");
        }
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

void readRequestHeader(rio_t *rp){
    char buf[MAXBUFFER];
    rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")){
        //stop when header == \r\n
        rio_readlineb(rp, buf, MAXLINE);
        cout << "[Request Header] " << buf;
    }
}

void getFileType(char* filename, char* filetype){
    if(strstr(filename, ".html")){
        strcpy(filetype, "text/html");
    }
    else if(strstr(filename,".gif")){
        strcpy(filetype, "image/gif");
    }
    else if(strstr(filename,".jpg")){
        strcpy(filetype, "image/jpeg");
    }
    else{
        //plain text
        strcpy(filetype, "text/plain");
    }
}

//send a http response, body include local file
void serve_static(int connfd, char* filename, int filesize){
    char buf[MAXBUFFER], filetype[MAXLINE];
    getFileType(filename, filetype);
    //send response header to client
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n",buf, filetype);
    rio_writen(connfd, buf, strlen(buf));
    //send response body to client
    char *srcptr;
    int srcfd;
    srcfd = open(filename, O_RDONLY, 0); //open for reading only
    srcptr = (char *)Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); //creates a new mapping in the virtual address space of the calling process
    //Call mmap to map the first filesize bytes of file srcfd to a private read-only memory area starting at address srcptr
    Close(srcfd);
    rio_writen(connfd, srcptr, filesize); //Do the file transfer to client, copy filesize bytes from the srcptr position to the client's connected fd
    Munmap(srcptr, filesize); //Free the mapped virtual memory area to avoid memory leaks
}

// Fork a child process and run a CGI program in the context of 
// the child process to serve various types of dynamic content
void serve_dynamic(int connfd, char* filename, char* cgi_args){
    char buf[MAXBUFFER];
    char *emptyList[] = { NULL };
    //send response header
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    rio_writen(connfd, buf, strlen(buf));
    sprintf(buf, "Server: Web Server\r\n");
    rio_writen(connfd, buf,strlen(buf));
    //fork child
    if(Fork() == 0){ // child process made
        setenv("QUERY_STRING", cgi_args, 1); //change or add environment variable
        Dup2(connfd, STDOUT_FILENO); //oldfd - newfd, redirect stdout to client
        Execve(filename, emptyList, environ); // execute CGI program
    } 
    Wait(NULL); //parent wait and repeat child
}

void clientError(int connfd, char* err, char* shortmsg, char* longmsg){
    char buffer[MAXBUFFER], body[MAXLINE];
    sprintf(body, "");
    sprintf(body, "%s%s: %s\r\n", body, err, shortmsg);
    sprintf(body, "%s%s\r\n", body, longmsg);
    sprintf(body, "%sError Message from Web Server\r\n",body);
    sprintf(buffer, "HTTP/1.0 %s %s\r\n",err,shortmsg);
    rio_writen(connfd, buffer, strlen(buffer));
    sprintf(buffer, "Content-type: text/html\r\n");
    rio_writen(connfd, buffer, strlen(buffer));
    sprintf(buffer, "Content-length: %d\r\n\r\n", (int)strlen(body));
    rio_writen(connfd, buffer, strlen(buffer));
    rio_writen(connfd, body, strlen(body));
}

