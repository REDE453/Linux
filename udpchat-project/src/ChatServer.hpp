#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <string>

#include "MsgPool.hpp"
#include "LogSvr.hpp"
#include "ConnectInfo.hpp"
#include "UserManager.hpp"
#include "Message.hpp"

#define UDP_PORT 17777
#define TCP_PORT 17778
#define THREAD_COUNT 2

//当前类实现
//1.接收客户端数据
//2.发送数据消息给客户端
//依赖UDP协议进行实现
class ChatServer
{
    public:
        ChatServer()
        {
            UdpSock_ = -1;
            UdpPort_ = UDP_PORT;
            MsgPool_ = NULL;
            TcpSock_ = -1;
            TcpPort_ = TCP_PORT;
            UserMana_ = NULL;
        }

        ~ChatServer()
        {
            if(MsgPool_)
            {
                delete MsgPool_;
                MsgPool_ = NULL;
            }

            if(UserMana_)
            {
                delete UserMana_;
                UserMana_ = NULL;
            }
        }

        //上次调用InitServer函数来初始化UDP服务
        void InitServer()
        {
            //1.创建UDP套接字
            UdpSock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(UdpSock_ < 0)
            {
                LOG(FATAL, "Create socket failed") << std::endl;
                exit(1);
            }
            //2.绑定地址信息
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(UdpPort_);
            addr.sin_addr.s_addr = inet_addr("0.0.0.0");

            int ret = bind(UdpSock_, (struct sockaddr*)&addr, sizeof(addr));
            if(ret < 0)
            {
                LOG(FATAL,"Bind addrinfo failed") << "0.0.0.0:17777" << std::endl;
                exit(2);
            }

            LOG(INFO, "Udp bind success") << "0.0.0.0:17777" << std::endl;
            //初始化数据池
            MsgPool_ = new MsgPool();
            if(!MsgPool_)
            {
                LOG(FATAL, "Create MsgPool failed") << std::endl;
                exit(3);
            }
            LOG(INFO, "Create MsgPool success") << std::endl;

            UserMana_ = new UserManager();
            if(!UserMana_)
            {
                LOG(FATAL, "Create user manager failed") << std::endl;
                exit(8);
            }

            //创建TCP-socket
            TcpSock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(TcpSock_ < 0)
            {
                LOG(FATAL, "Create tcp socket failed") << std::endl;
                exit(5);
            }

            int opt = 1;
            setsockopt(TcpSock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            struct sockaddr_in tcpaddr;
            tcpaddr.sin_family = AF_INET;
            tcpaddr.sin_port = htons(TcpPort_);
            tcpaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
            ret = bind(TcpSock_, (struct sockaddr*)&tcpaddr, sizeof(tcpaddr));
            if(ret < 0)
            {
                LOG(FATAL, "Bind Tcp addrinfo failed") << std::endl;
                exit(6);
            }

            ret = listen(TcpSock_, 5);
            if(ret < 0)
            {
                LOG(FATAL, "Tcp listen failed") << std::endl;
                exit(7);
            }
            LOG(INFO, "Tcp listen 0.0.0.0:17778") << std::endl;
        }

        //初始化程序当中生产和消费线程
        void Start()
        {
            pthread_t tid;
            for(int i = 0; i < THREAD_COUNT; i++)
            {
                int ret = pthread_create(&tid, NULL, ProductMsgStart, (void*)this);
                if(ret < 0)
                {
                    LOG(FATAL, "pthread_create new thread failed") << std::endl;
                    exit(4);
                }
                
                ret = pthread_create(&tid, NULL, ConsumeMsgStart, (void*)this);
                if(ret < 0)
                {
                    LOG(FATAL, "pthread_create new thread failed") << std::endl;
                    exit(4);
                }
            }
            LOG(INFO, "UdpChat Service start successs") << std::endl;

            while(1)
            {
                struct sockaddr_in cliaddr;
                socklen_t cliaddrlen = sizeof(cliaddr);
                int newsock = accept(TcpSock_, (struct sockaddr*)&cliaddr, &cliaddrlen);
                if(newsock < 0)
                {
                    LOG(ERROR, "Accept new connect failed") << std::endl;
                    continue;
                }

                LoginConnect* lc = new LoginConnect(newsock, (void*)this);
                if(!lc)
                {
                    LOG(ERROR, "Create LoginConnect failed") << std::endl;
                    continue;
                }
                //创建线程，处理登录，注册的请求
                pthread_t tid;
                int ret = pthread_create(&tid, NULL, LoginRegStart, (void*)lc);
                if(ret < 0)
                {
                    LOG(ERROR, "Create User LoginConnect thread failed") << std::endl;
                    continue;
                }
                LOG(INFO, "Create TcpConnect thread successs") << std::endl;
            }
        }
    private:
        static void* ProductMsgStart(void* arg)
        {
            pthread_detach(pthread_self());
            ChatServer* cs = (ChatServer*)arg;
            while(1)
            {
                //recvfrom udp数据
                cs->RecvMsg();
            }
            return NULL;
        }

        static void* ConsumeMsgStart(void* arg)
        {
            pthread_detach(pthread_self());
            ChatServer* cs = (ChatServer*)arg;
            while(1)
            {
                //sendto udp
                cs->BroadcastMsg();
            }
            return NULL;
        }

        static void* LoginRegStart(void* arg)
        {
            pthread_detach(pthread_self());
            LoginConnect* lc = (LoginConnect*)arg;
            ChatServer* cs = (ChatServer*)lc->GetServer();
            //注册, 登录请求
            //    请求从cli端来，recv(sock, buf, size, 0);
            char QuesType;
            ssize_t recvsize = recv(lc->GetTcpSock(), &QuesType, 1, 0);
            if(recvsize < 0)
            {
                LOG(ERROR, "Recv TagType failed") << std::endl;
                return NULL;
            }
            else if(recvsize == 0)
            {
                LOG(ERROR, "Client shutdown connect") << std::endl;
                return NULL;
            }
            uint32_t UserId = -1;
            int UserStatus = -1;

            //正常接收到一个请求标识
            switch(QuesType)
            {
                case REGISTER:
                    //使用用户管理模块的注册
                    UserStatus = cs->DealRegister(lc->GetTcpSock(), &UserId);
                    break;
                case LOGIN:
                    //使用用户管理模块的登录
                    {
                        UserStatus = cs->DealLogin(lc->GetTcpSock());
                        printf("UserStatus:%d\n", UserStatus);
                        break;
                    }
                case LOGINOUT:
                    //使用用户管理模块的退出登录
                    cs->DealLoginOut();
                    break;
                default:
                    UserStatus = REGFAILED;
                    LOG(ERROR, "Recv Request type not a effective value") << std::endl;
                    break;
            }

            //    响应 send(sock, buf, size, 0);
            ReplyInfo ri;
            ri.Status = UserStatus;
            ri.UserId_ = UserId;
            ssize_t sendsize = send(lc->GetTcpSock(), &ri, sizeof(ri), 0);
            if(sendsize < 0)
            {
                //TODO
                //如果发送数据失败了，是否需要考虑应用层重新发送一个
                LOG(ERROR, "SendMsg Failed") << std::endl;
            }
            LOG(INFO, "SendMsg success") << std::endl;

            //将TCP连接释放掉
            close(lc->GetTcpSock());
            delete lc;
            return NULL;
        }
        k

        int DealRegister(int Sock, uint32_t* UserId)
        {
            //接收注册请求
            RegInfo ri;
            ssize_t recvsize = recv(Sock, &ri, sizeof(ri), 0);
            if(recvsize < 0)
            {
                LOG(ERROR, "Recv TagType failed") << std::endl;
                return OFFLINE;
            }
            else if(recvsize == 0)
            {
                LOG(ERROR, "Client shutdown connect") << std::endl;
                //特殊处理对端关闭的情况;
            }

            //调用用户管理模块进行注册请求的处理
            //返回注册成功之后给用户的UserId
            int ret = UserMana_->Register(ri.NickName_, ri.School_, ri.Passwd_, UserId);
            //返回当前状态
            if(ret == -1)
            {
                return REGFAILED;
            }

            return REGISTERED;
        }

        int DealLogin(int Sock)
        {
            struct LoginInfo li;
            ssize_t recvsize = recv(Sock, &li, sizeof(li), 0);
            if(recvsize < 0)
            {
                LOG(ERROR, "Recv TagType failed") << std::endl;
                return OFFLINE;
            }
            else if(recvsize == 0)
            {
                LOG(ERROR, "Client shutdown connect") << std::endl;
                //需要在处理下
            }

            LOG(DEBUG, "UserId:Passwd") << li.UserId_ << ":" << li.Passwd_ << std::endl;
            int ret = UserMana_->Login(li.UserId_, li.Passwd_);
            if(ret == -1)
            {
                LOG(ERROR, "User Login failed") << std::endl;
                return LOGINFAILED;
            }
            return LOGINED;
        }

        int DealLoginOut()
        {

        }
    private:
        void RecvMsg()
        {
            char buf[10240] = {0}; 
            struct sockaddr_in cliaddr;
            socklen_t cliaddrlen = sizeof(struct sockaddr_in);
            int recvsize = recvfrom(UdpSock_, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&cliaddr,&cliaddrlen);
            if(recvsize < 0)
            {
                LOG(ERROR, "recvfrom msg failed") << std::endl;
            }
            else
            {
                //正常逻辑
                std::string msg;
                msg.assign(buf, recvsize);
                LOG(INFO, msg) << std::endl;
                //需要将发送的数据从JSON格式转化为我们可以识别的数据
                Message jsonmsg;
                jsonmsg.Deserialize(msg);
                //需要增加用户管理，只有注册登录的人才可以向服务器发送消息
                //1.校验当前的消息是否属注册用户或者老用户发送的
                //    1.1 不是，则认为是非法消息
                //    1.2 是 区分一下是否第一次发送消息
                //      是：保存地址信息，并且更新状态为在线，将数据放到数据池当中
                //      是老用户：直接将数据放到数据池当中
                //2.需要校验，势必是和用户管理模块打交道
                bool ret = UserMana_->IsLogin(jsonmsg.GetUserId(), cliaddr, cliaddrlen);
                if(ret != true)
                {
                    LOG(ERROR, "discarded the msg ") << msg << std::endl;
                }

                LOG(INFO, "Push msg to MsgPool") << std::endl;
                MsgPool_->PushMsgToPool(msg);
            }
        }

        void BroadcastMsg()
        {
            //1.获取发送的内容
            std::string msg;
            MsgPool_->PopMsgFromPool(&msg);
            //2.获取要给哪一个用户进行发送
            //用户管理系统提供结构获取在线的用户列表 
            std::vector<UserInfo> OnlineUserVec;
            UserMana_->GetOnlineUserInfo(&OnlineUserVec);
            std::vector<UserInfo>::iterator iter = OnlineUserVec.begin();
            for(; iter != OnlineUserVec.end(); iter++)
            {
                //SendMsg(msg, udp地址信息，udp地址信息长度);--》UserInfo
                SendMsg(msg, iter->GetCliAddrInfo(), iter->GetCliAddrLen());
            }
        }

        //给一个客户端发送单个消息接口
        void SendMsg(const std::string& msg, struct sockaddr_in& cliaddr, socklen_t& len)
        {
            ssize_t sendsize = sendto(UdpSock_, msg.c_str(), msg.size(), 0, (struct sockaddr*)&cliaddr, len);
            if(sendsize < 0)
            {
                LOG(ERROR, "sendto msg failed") << std::endl;
                //没有发送成功，我们是否需要缓存没有发送成功的信息还有客户端的地址
            }
            else
            {
                //成功
                LOG(INFO, "Sendto msg success ") << "[" << inet_ntoa(cliaddr.sin_addr) << ":" << ntohs(cliaddr.sin_port) << "]" << msg << std::endl;
            }
        }
    private:
        int UdpSock_;
        int UdpPort_;
        //数据池
        MsgPool* MsgPool_;

        //tcp处理注册，登录请求
        int TcpSock_;
        int TcpPort_;
        //用户管理
        UserManager* UserMana_;
};
