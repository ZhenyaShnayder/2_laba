#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <list>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t wasSigHup = 0;
void sigHupHandler(int r){
    wasSigHup = 1;
}

using namespace std;
int main(){
    cout<<"pid "<<getpid()<<endl;
    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);

    //блокировка сигнала
    sigset_t blockedMask, origMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

    int super_socket=socket(AF_INET, SOCK_STREAM, 0);
    list <int> accepted_fd;
    if(super_socket<0){
        perror("super_socket failed ");
        exit(1);
    }
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(8888);
    addr.sin_addr.s_addr=INADDR_ANY;
    if(bind(super_socket, (const struct sockaddr*)&addr, sizeof(addr))<0){
        perror("bind failed ");
        exit(2);
    }
    if(listen(super_socket, 5)<0){
        perror("listen failed ");
        exit(3);
    }
    fd_set mask_read;

    bool connect=true;
    while(connect){
        FD_ZERO(&mask_read);
        int max_fd=super_socket;
        FD_SET(super_socket, &mask_read);
        for(list <int>::iterator i=accepted_fd.begin(); i!=accepted_fd.end(); i++){
            max_fd=(*i<max_fd)?max_fd:*i;
            FD_SET(*i, &mask_read);
        }

        if(pselect(max_fd+1, &mask_read, nullptr, nullptr, 0, &origMask)<0){
            if(errno==EINTR&&wasSigHup==1){
                cout<<"wasSigHup\n";
                wasSigHup=0;
                continue;
            }
            perror("pselect failed ");
            exit(4);
        }

        if(FD_ISSET(super_socket, &mask_read)){
            accepted_fd.push_back(0);
            struct sockaddr_in client_addr;
            socklen_t len;
            if((accepted_fd.back()=accept(super_socket, (struct sockaddr*)&client_addr, &len))<0){
                perror("accepted failed ");
                exit(4);
            }
            cout<<"Client was connected"<<endl;
            if(accepted_fd.size()>1){
                close(accepted_fd.back());
                accepted_fd.pop_back();
                cout<<"Client was disconnected by server"<<endl;
            }
        }
        char buffer[512];
        for(list <int>::iterator i=accepted_fd.begin(); i!=accepted_fd.end(); i++){
            if(FD_ISSET(*i, &mask_read)){
                int number=read(*i, buffer, 512);
                if(number==0){
                    close(*i);
                    accepted_fd.erase(i);
                    cout<<"Client was disconnected"<<endl;
                    if(accepted_fd.size()==0)
                        connect=false;
                    break;
                }
                cout<<"read: "<<number<<" bytes\n";
            }
        }
    }

}