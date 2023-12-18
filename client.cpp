#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int main(){
    int client_socket=socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket<0){
        perror("client_socket failed ");
        exit(1);
    }
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(8888);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    if(connect(client_socket, (const struct sockaddr*)&addr, sizeof(addr))<0){
        perror("connect failed ");
        exit(2);
    }
    char mess[20]="I wanna communicate";
    send(client_socket, mess, 20, 0);
    string str;
    cin>>str;
    close(client_socket);
}