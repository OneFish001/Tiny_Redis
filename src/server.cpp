// #include "server.h"

#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>

#define PORT 6379
#define BUFFER_SIZE 4096

//定义一个库
std::unordered_map<std::string,std::string>  db;


// 处理客户请求
void process_command(int client_socket){

    char buffer[BUFFER_SIZE] ={0};
    while(true){
        int bytes_read=read(client_socket,buffer,BUFFER_SIZE);
        if(bytes_read<=0){
            //客户端断开连接
            std::cout<<"客户端断开连接"<<std::endl;
            break;

        }
        
        std::stringstream ss(buffer);
        std::string cmd;
        ss >> cmd;

        if(cmd=="SET"){
            std::string key,value;
            ss>>key>>value;
            db[key]=value;
            send(client_socket,"OK\n",3,0);
       }else if(cmd=="GET"){
            std::string key;
            ss>>key;
            auto it=db.find(key);
            if(it!=db.end()){
                // std::string response=it->second+'\n';
                send(client_socket,(it->second+'\n').c_str(),(it->second).size()+1,0);
            }else{
                send(client_socket,"nil\n",6,0);
            }
        }else{
                send(client_socket,"未知命令\n",13,0);
    }
     
    //清空缓冲区
    memset(buffer,0,BUFFER_SIZE);



    }
    // read(client_socket,buffer,BUFFER_SIZE);
   

//     std::stringstream ss(buffer);
//     std::string cmd;
//     ss >> cmd;
//     if(cmd=="SET"){
//         std::string key,value;
//         ss>>key>>value;
//         db[key]=value;
//         send(client_socket,"OK\n",3,0);
//    }else if(cmd=="GET"){
//         std::string key;
//         ss>>key;
//         auto it=db.find(key);
//         if(it!=db.end()){
//             // std::string response=it->second+'\n';
//             send(client_socket,(it->second+'\n').c_str(),(it->second).size()+1,0);
//         }else{
//             send(client_socket,"nil\n",6,0);
//         }
//     }else{
//             send(client_socket,"未知命令\n",13,0);
// }
   close(client_socket);
}



int main(){
    //创建TCP套接字
    int server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd<0){
        std::cout<<"无法创建套接字！"<<std::endl;
        return 1;
    }
    

    //绑定地址和端口
    sockaddr_in address;
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(PORT);

    if(bind(server_fd,(sockaddr*)&address,sizeof(address))<0){
        std::cerr<<"绑定失败了！"<<std::endl;
        return 1;
    }

    //开始监听
    if(listen(server_fd,3)<0){
        std::cerr<<"监听失败了！"<<std::endl;
        return 1;

    }

    std::cout<<"服务器启动，监听接口："<<PORT<<std::endl;

    //循环接受客户端的连接申请
    while(true){
        int clients_socket=accept(server_fd,nullptr,nullptr);
        if(clients_socket<0){
            std::cerr<<"连接失败啦！"<<std::endl;
            break;
        }else{

            //实现命令解析
            process_command(clients_socket);
            // close(clients_socket);

        }

        //暂时处理单个连接
        // break;
    }
    
   
    
    
    //关闭服务器
    close(server_fd);
    return 0;

}