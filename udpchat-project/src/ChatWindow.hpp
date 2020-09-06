#pragma once
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include <vector>

#include "ChatClient.hpp"
#include "Message.hpp"

class ChatWindow;

class Param
{
    public:
        Param(ChatWindow* winp, int threadnum, ChatClient* chatcli)
        {
            winp_ = winp;
            thread_number_ = threadnum;
            chatcli_ = chatcli;
        }
    public:
        ChatWindow* winp_;
        int thread_number_;

        ChatClient* chatcli_;
};


class ChatWindow
{
    public:
        ChatWindow()
        {
            header_ = NULL;
            output_ = NULL;
            user_list_ = NULL;
            input_ = NULL;
            threads_.clear();
            pthread_mutex_init(&lock_, NULL);
            //初始化屏幕，启动CURSES模式， 必须用endwin结束
            initscr();
            //这个函数用来设制光标是否可见。它的参数可以是：0（不可见），1（可见），2（完全可见）
            curs_set(0);
        }

        ~ChatWindow()
        {
            if(header_)
            {
                //删除窗口
                delwin(header_);
            }

            if(output_)
            {
                delwin(output_);
            }

            if(user_list_)
            {
                delwin(user_list_);
            }

            if(input_)
            {
                delwin(input_);
            }
            endwin();
            pthread_mutex_destroy(&lock_);
        }
        void DrawHeader()
        {
            int lines = LINES / 5;
            int cols  = COLS;
            int start_y = 0;
            int start_x = 0;
            //打开一个新的窗口， 行，列， 纵坐标，横坐标
            header_ = newwin(lines, cols, start_y, start_x);
            box(header_, 0, 0);
            pthread_mutex_lock(&lock_);
            wrefresh(header_);
            pthread_mutex_unlock(&lock_);
        }

        void DrawOutput()
        {
            int lines = (LINES * 3) / 5;
            int cols  = (COLS * 3) / 4;
            int start_y = LINES / 5;
            int start_x = 0;
            output_ = newwin(lines, cols, start_y, start_x);
            box(output_, 0, 0);
            pthread_mutex_lock(&lock_);
            wrefresh(output_);
            pthread_mutex_unlock(&lock_);
        }

        void DrawUserList()
        {
            int lines = (LINES * 3) / 5;
            int cols  = COLS / 4;
            int start_y = LINES / 5;
            int start_x = (COLS * 3) / 4;
            user_list_ = newwin(lines, cols, start_y, start_x);
            //绘制窗口
            box(user_list_, 0, 0);
            pthread_mutex_lock(&lock_);
            //刷新窗口
            wrefresh(user_list_);
            pthread_mutex_unlock(&lock_);
        }

        void DrawInput()
        {
            int lines = LINES / 5;
            int cols  = COLS;
            int start_y = LINES*4/5;
            int start_x = 0;
            input_ = newwin(lines, cols, start_y, start_x);
            box(input_, 0, 0);
            pthread_mutex_lock(&lock_);
            wrefresh(input_);
            pthread_mutex_unlock(&lock_);
        }

        //y 对应 行
        //x 对应 列    数轴当中坐标
        void PutStringToWin(WINDOW* win, int y, int x, std::string& msg)
        {
            //将msg输出到win当中
            mvwaddstr(win, y, x, msg.c_str());
            pthread_mutex_lock(&lock_);
            wrefresh(win);
            pthread_mutex_unlock(&lock_);
        }

        void GetStringFromWin(WINDOW* win, std::string* Data)
        {
            char buf[1024];
            memset(buf, '\0', sizeof(buf));
            //从窗口当中获取n个字符
            wgetnstr(win, buf, sizeof(buf) - 1);
            (*Data).assign(buf, strlen(buf));
        }

        static void RunHeader(ChatWindow* cw)
        {
            //1.绘制窗口
            //2.展示欢迎语
            int y, x;
            size_t pos = 1;
            //标识是否需要改变输出的方向  本身是想右输出的，到最右边之后，想左输出
            //0：可以继续向右输出
            //1: 需要向左输出
            int flag = 0;
            std::string msg = "Welcome to our chat system";
            while(1)
            {
                cw->DrawHeader();
                //getmaxyx函数返回窗口最大的行数存储在变量y当中，返回最大的列数存储在变量x当中
                getmaxyx(cw->header_, y, x);
                cw->PutStringToWin(cw->header_, y / 2, pos, msg);
                //左边的边界
                if(pos < 2)
                {
                    flag = 0;
                }
                else if(pos > x - msg.size() - 2) //右边界
                {
                    flag = 1;
                }

                if(flag == 0)
                {
                    pos++;
                }
                else
                {
                    pos--;
                }
                sleep(1);
            }
        }

        static void RunOutput(ChatWindow* cw, ChatClient* cc)
        {
            std::string recv_msg;
            Message msg;
            cw->DrawOutput();
            int line = 1;
            int y, x;
            while(1)
            {
                getmaxyx(cw->output_, y, x);
                cc->RecvMsg(&recv_msg);
                //反序列化
                msg.Deserialize(recv_msg);
                //展示数据 昵称-school# 消息
                std::string show_msg;
                show_msg += msg.GetNickName();
                show_msg += "-";
                show_msg += msg.GetSchool();
                show_msg += "# ";
                show_msg += msg.GetMsg();
                if(line > y - 2)
                {
                    line = 1;
                    cw->DrawOutput();
                }

                cw->PutStringToWin(cw->output_, line, 1, show_msg);
                line++;

                std::string user_info;
                user_info += msg.GetNickName();
                user_info += "-";
                user_info += msg.GetSchool();

                cc->PushUser(user_info);
            }
        }
        static void RunInput(ChatWindow* cw, ChatClient* cc)
        {
            //昵称 学校 msg 用户id
            Message msg;
            //设置昵称
            msg.SetNickName(cc->GetMySelf().NickName_);
            msg.SetSchool(cc->GetMySelf().School_);
            msg.SetUserId(cc->GetMySelf().UserId_);
            //用户输入的原始消息
            std::string user_enter_msg;
            //序列化完成之后的消息
            std::string send_msg;
            std::string tips = "Please Enter# ";
            while(1)
            {
                cw->DrawInput();
                cw->PutStringToWin(cw->input_, 2, 2, tips);
                //从窗口当中获取数据，放到send_msg当中
                cw->GetStringFromWin(cw->input_, &user_enter_msg);
                msg.SetMsg(user_enter_msg);

                msg.serialize(&send_msg);
                cc->SendMsg(send_msg);
            }
        }
        static void RunUserList(ChatWindow* cw, ChatClient* cc)
        {
            int y, x;
            while(1)
            {
                cw->DrawUserList();
                getmaxyx(cw->user_list_, y, x);
                std::vector<std::string> UserList = cc->GetOnlineUser();
                for(auto& iter:UserList)
                {
                    int line = 1;
                    cw->PutStringToWin(cw->user_list_, line++, 1, iter);
                }
                sleep(1);
            }
        }

        static void* DrawWindow(void* arg)
        {
            //int thread_num = 0/1/2/3;
            Param* pm = (Param*)arg;
            ChatWindow* cw = pm->winp_;
            ChatClient* cc = pm->chatcli_;
            int thread_num = pm->thread_number_;
            switch(thread_num)
            {
                case 0:
                    //改造
                    RunHeader(cw);
                    break;
                case 1:
                    RunOutput(cw, cc);
                    break;
                case 2:
                    RunUserList(cw, cc);
                    break;
                case 3:
                    RunInput(cw, cc);
                    break;
                default:
                    break;
            }
            delete pm;
            return NULL;
        }

        void Start(ChatClient* chatcli)
        {
            int i = 0;
            pthread_t tid;
            for(; i < 4; i++)
            {
                Param* pm = new Param(this, i, chatcli);
                int ret = pthread_create(&tid, NULL, DrawWindow, (void*)pm);
                if(ret < 0)
                {
                    printf("Create thread failed\n");
                    exit(1);
                }
                threads_.push_back(tid);
            }

            for(i = 0; i < 4; i++)
            {
                pthread_join(threads_[i], NULL);
            }
        }
    private:
        WINDOW* header_;
        WINDOW* output_;
        WINDOW* user_list_;
        WINDOW* input_;

        std::vector<pthread_t> threads_;
        pthread_mutex_t lock_;
};
