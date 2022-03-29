#include "Network.hpp"
#include "RequestHandler.hpp"
using namespace std;
int main(int argc, char** argv){
    Network network;
    int connfd;
    //listen to port which is my last 4 digit number of student ID
    //due to my last 4 digit number is 0677 so i change 0 -> 3 (which is my first digit number of student ID)
    //student ID : 3190300677
    network.Listen(3677);
    cout << "[Server] Tiny Web Server Begin" << endl;
    cout << "[Server] Waiting For Request..." << endl;
    while(1){ //non stop loop for accept request and handle the request
        connfd = network.Acccept(); //accept request
        //then handle request
        RequestHandler rh(connfd);
        cout << "[Server] Connection Established! Socket ID : " << connfd << endl;
        rh.Begin();
        network.Close();
    }
    return 0;
}
