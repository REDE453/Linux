#pragma once
#include <iostream>

#define REGISTER 0
#define LOGIN 1
#define LOGINOUT 2

//约定双发发送的UDP数据包最大多大
#define MESSAGE_MAX_SIZE 1024

//注册信息
struct RegInfo
{
    char NickName_[15];
    char School_[20];
    char Passwd_[20];
};

//登录信息
struct LoginInfo
{
    uint32_t UserId_; //返回给用户的ID号
    char Passwd_[20];
};

enum USerStatus
{
    REGFAILED = 0, //注册失败
    REGISTERED,    //注册成功
    LOGINFAILED,   //登录失败
    LOGINED        //登录成功
};

//应答信息
struct ReplyInfo
{
    //当前状态, 注册完成，登录完成 
    int Status;
    uint32_t UserId_;
};

class LoginConnect
{
    public:
        LoginConnect(int Sock, void* Server)
        {
            Sock_ = Sock;
            Server_ = Server;
        }
        int GetTcpSock()
        {
            return Sock_;
        }

        void* GetServer()
        {
            return Server_;
        }
    private:
        int Sock_;
        //可以保存ChatServer类的实例化指针
        void* Server_;
};
