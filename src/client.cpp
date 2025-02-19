#include "../include//client.h"
// #include <iostream>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>

// #define PORT 6379

int main(){
    int sock=socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in serv_addr;
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(PORT);

    //连接本地服务器
    inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr);

    if(connect(sock,(sockaddr*)&serv_addr,sizeof(serv_addr))){
        std::cerr<<"连接失败！"<<std::endl;
        return 1;
    }

    //循环处理
    while(true){
        std::cout<<">";
        std::string cmd;
        std::getline(std::cin,cmd);
        cmd.erase(std::remove(cmd.begin(),cmd.end(),'\n'),cmd.end());//去除换行符


        //发送命令
        send(sock,cmd.c_str(),cmd.size(),0);

        if(cmd=="exit"){
            std::cout<<"不发送信息了！"<<std::endl;
            break;
        }

        //接受响应
      
        char buffer[BUFFER_SIZE] ={0};
        int len = read(sock,buffer,sizeof(buffer));
        
        if(len>0){
            buffer[len]='\0';
            std::cout<<"服务器返回："<<buffer<<std::endl;
        }else if(len==0){
            std::cerr<<"服务器关闭连接！"<<std::endl;
        }
        else{
            std::cerr<<"读取服务器返回的数据失败！"<<std::endl;
            break;
        }
    }

    close(sock);
    return 0;

}
