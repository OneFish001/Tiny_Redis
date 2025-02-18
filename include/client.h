#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 6379

int main(){
    int sock=socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in serv_addr;
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(PORT);

    //连接本地服务器
    inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr);

    if(connect(sock,(socket*)&serv_addr,sizeof(serv_addr)){
        std::cerr<<"连接失败！"<<std::endl;
        return 1;
    }

    //循环处理
    while(true){
        std::cout<<">";
        std::string cmd;
        std::getline(std::cin,cmd);


        //发送命令
        send(sock,cmd.c_str(),cmd.size(),0);

        //接受响应
        char buffer[1024] ={0};
        read(sock,buffer,1024);
        std::cout<<"服务器返回："<<buffer;
    }

    close(sock);
    return 0;

}
