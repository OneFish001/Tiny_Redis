#include "../include//server.h"
#include "../include/AOF_logger.h"


LRUCache db(100);//实例化最大缓存
AOFLogger aof_logger("appendonly.aof");//AOF日志实例


// 处理客户请求
void process_command(int client_socket){

    char buffer[BUFFER_SIZE] ={0};
    while(true){
        int bytes_read=read(client_socket,buffer,BUFFER_SIZE);
        if(bytes_read<=0){
            std::cout<<"客户端断开连接"<<std::endl;
        }

        std::stringstream ss(buffer);
        std::string cmd;
        ss >> cmd;

        if(cmd=="SET"){
            std::string key,value;
            ss>>key>>value;
            
            db.put(key,value);
            aof_logger.append(std::string("SET")+" "+key+" "+value);
            std::cout<<"SET"<<key<<" "<<value<<std::endl;
            send(client_socket,"OK\n",3,0);

       }else if(cmd=="GET"){
            std::string key;
            ss>>key;
            std::string value=db.get(key);
            send(client_socket, value.c_str(), value.size(), 0);
        }else{
                send(client_socket,"未知命令\n",13,0);
    }
     
    //清空缓冲区
    memset(buffer,0,BUFFER_SIZE);

    }
   close(client_socket);
}





//初始化全局线程库
ThreadPool pool(4);

//线程池构造函数
ThreadPool::ThreadPool(size_t num_threads):stop(false){
    for(int i=0;i<num_threads;i++){
        threads.__emplace_back([this]{
            while(true){
                int client_socket;
                {
                    //等待新任务或停止信号
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    condition.wait(lock,[this]{
                        return stop||!tasks.empty();
                    });

                    if( stop && tasks.empty()) return ;//任务为空，就返回

                    client_socket=tasks.front();
                    tasks.pop();

                }
                process_command(client_socket);
            }

        });
    }

}

//添加任务到队列
void ThreadPool::enqueue(int client_socket){
    std::unique_lock<std::mutex>  lock(queue_mutex);
    tasks.push(client_socket);

    //通知下一个等待线程
    condition.notify_one();
}

//析构函数
ThreadPool::~ThreadPool(){
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop=true;

    //唤醒所有线程退出
    condition.notify_all();
    for(auto &thread:threads){
        thread.join();//等待所有线程结束
    }
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

    //开始绑定
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

            pool.enqueue(clients_socket);

        }

    }
    
   
    
    
    //关闭服务器
    close(server_fd);
    return 0;

}