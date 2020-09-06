#include "ChatClient.hpp"
#include "ChatWindow.hpp"


void Menu()
{
    std::cout << "-------------------------------" << std::endl;
    std::cout << "| 1.register           2.login|" << std::endl;
    std::cout << "                               " << std::endl;
    std::cout << "| 3.logout             4.exit |" << std::endl;
    std::cout << "-------------------------------" << std::endl;
}


int main(int argc, char* argv[])
{
    //./Cli [ip]
    if(argc != 2)
    {
        printf("./Cli [ip]\n");
        exit(1);
    }

    ChatClient* cc = new ChatClient(argv[1]);
    //1.初始化服务
    cc->Init();
    while(1)
    {
        Menu();
        int Select = -1;
        std::cout << "Please select service: ";
        fflush(stdout);
        std::cin >> Select;
        if(Select == 1)
        {
            //注册
            if(!cc->Register())
            {
                std::cout << "Register failed! Please Try Again!" << std::endl;
            }
            else
            {
                std::cout << "Register success! Please login!" << std::endl;
            }
        }
        else if(Select == 2)
        {
            //登录
            if(!cc->Login())
            {
                std::cout << "login failed! Please check Your UserId or Password" << std::endl;
            }
            else
            {
                std::cout << "login success! Please Chat Online!" << std::endl;
                ChatWindow* cw = new ChatWindow();
                cw->Start(cc);
                sleep(5);
            }

        }
        else if(Select == 3)
        {
            //退出登录
        }
        else if(Select == 4)
        {
            //退出
            break;
        }
    }
    delete cc;
    return 0;
}
