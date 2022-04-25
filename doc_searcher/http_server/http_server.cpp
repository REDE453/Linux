#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "cpp-httplib-master/httplib.h"
#include "../searcher/searcher.h"

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using std::vector;

int main() 
{
    using namespace httplib;
    // 1. 创建 Searcher 对象
    searcher::Searcher searcher;
    bool ret = searcher.Init("../data/tmp/raw_input");
    if (!ret) 
    {
        std::cout << "Searcher 初始化失败" << std::endl;
        return 1;
    }

    // 2. 创建 server 对象
    Server server;
    server.Get("/searcher", [&searcher](const Request& req, Response& resp) {
        if (!req.has_param("query")) {
            resp.set_content("您发的请求参数错误", "text/plain; charset=utf-8");
            return;
        }
        string query = req.get_param_value("query");
        string results;
        searcher.Search(query, &results);
        resp.set_content(results, "application/json; charset=utf-8");
    });
    // 告诉服务器, 静态资源存放在 wwwroot 目录下. (html, css, js, 图片....)
    // 服务器启动之后, 可以通过 http://127.0.0.1:10001/index.html 来访问到该页面了
    server.set_base_dir("./wwwroot");
    // 3. 启动服务器
    server.listen("0.0.0.0", 10001);
    return 0;
}
