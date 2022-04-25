// 预处理模块.
// 功能就是要读取并分析 boost 文档的 .html 内容,
// 解析出每个文档的标题, url, 正文(去除 html 标签)
// 最终把结果输出成一个 行文本 文件
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "../common/util.hpp"

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using std::vector;

// 这个变量表示从哪个目录中读取 boost 文档的 html
string g_input_path = "../data/input/";

// 这个变量对应预处理模块输出结果放到哪里
string g_output_path = "../data/tmp/raw_input";

// 创建一个结构体, 表示一个文档(一个 HTML)
struct DocInfo 
{
    // 文档的标题
    string title;
    // 文档的 url
    string url;
    // 文档的正文
    string content;
};

bool EnumFile(const string &input_path, vector<string> *file_list) 
{
    // 枚举目录需要使用 boost 来完成.
    // 把 boost::filesystem 这个命名空间定义一个别名.
    namespace fs = boost::filesystem;
    fs::path root_path(input_path);
    if (!fs::exists(root_path)) 
    {
        std::cout << "当前的目录不存在" << std::endl;
        return false;
    }
    fs::recursive_directory_iterator end_iter;
    for (fs::recursive_directory_iterator iter(root_path); iter != end_iter;++iter) 
    {
        // 当前的路径对应的是不是一个普通文件. 如果是目录, 直接跳过.
        if (!fs::is_regular_file(*iter)) 
        {
            continue;
        }
        // 当前路径对应的文件是不是一个 html 文件. 如果是其他文件也跳过.
        if (iter->path().extension() != ".html") 
        {
            continue;
        }
        // 把得到的路径加入到最终结果的 vector 中
        file_list->push_back(iter->path().string());
    }
    return true;
}

// 找到 html 中的 title 标签
bool ParseTitle(const string &html, string *title) 
{
    size_t beg = html.find("<title>");
    if (beg == string::npos) 
    {
        std::cout << "标题未找到" << std::endl;
        return false;
    }
    size_t end = html.find("</title>");
    if (end == string::npos) 
    {
        std::cout << "标题未找到" << std::endl;
        return false;
    }
    beg += string("<title>").size();
    if (beg >= end) 
    {
        std::cout << "标题位置不合法" << std::endl;
        return false;
    }
    *title = html.substr(beg, end - beg);
    return true;
}

// 根据本地路径获取到在线文档的路径.
// 本地路径形如:
// ../data/input/html/thread.html
// 在线路径形如:
// https://www.boost.org/doc/libs/1_53_0/doc/html/thread.html
// 把本地路径的后半部分截取出来, 再拼装上在线路径的前缀就可以了
bool ParseUrl(const string &file_path, string *url) 
{
    string url_head = "https://www.boost.org/doc/libs/1_53_0/doc/";
    string url_tail = file_path.substr(g_input_path.size());
    *url = url_head + url_tail;
    return true;
}

// 针对读取出来的 html 内容进行去标签.
bool ParseContent(const string &html, string *content) 
{
    bool is_content = true;
    for (auto c : html) 
    {
        if (is_content) 
        {
            // 当前已经是正文了
            if (c == '<') 
            {
                // 当前遇到了标签了.
                is_content = false;
            } else 
            {
                // 这里还要单独处理下 \n . 预期的输出结果是一个行文本文件.
                // 最终结果 raw_input 中每一行对应到一个原始的 html 文档
                // 此时就需要把 html 文档中原来的 \n 都干掉.
                if (c == '\n') 
                {
                    c = ' ';
                }
                // 当前是普通字符, 就把结果写入到 content 中
                content->push_back(c);
            }
        } else 
        {
            // 当前已经是标签了
            if (c == '>') 
            {
                // 标签结束
                is_content = true;
            }
            // 标签里的其他内容都直接忽略掉.
        }
    }
    return true;
}

bool ParseFile(const string &file_path, DocInfo *doc_info) 
{
    // 1. 先读取文件的内容
    //    把整个文件内容都读取出来.
    string html;
    bool ret = common::Util::Read(file_path, &html);
    if (!ret) 
    {
        std::cout << "解析文件失败! 读取文件失败!" << file_path << std::endl;
        return false;
    }
    // 2. 根据文件内容解析出标题. (html 中有一个 title 标签)
    ret = ParseTitle(html, &doc_info->title);
    if (!ret) 
    {
        std::cout << "解析标题失败!" << std::endl;
        return false;
    }
    // 3. 根据文件的路径, 构造出对应的在线文档的 url
    ret = ParseUrl(file_path, &doc_info->url);
    if (!ret) {
        std::cout << "解析 url 失败" << std::endl;
        return false;
    }
    // 4. 根据文件内容, 进行去标签, 作为 doc_info 中的 content 字段的内容
    ret = ParseContent(html, &doc_info->content);
    if (!ret) 
    {
        std::cout << "解析正文失败" << std::endl;
        return false;
    }
    return true;
}

// ofstream 类是没有拷贝构造函数的(不能拷贝的)
// 按照参数传的时候, 只能传引用或者指针.
// 此处还不能是 const 引用, 否则无法执行里面的写文件操作.
// 每个 doc_info 就是一行
void WriteOutput(const DocInfo &doc_info, std::ofstream &ofstream) 
{
    ofstream << doc_info.title << "\3" << doc_info.url << "\3"
             << doc_info.content << std::endl;
}

// 预处理过程
// 1. 把 input 目录中所有的 html 路径都枚举出来.
// 2. 根据枚举出来的路径依次读取每个文件内容, 并进行解析.
// 3. 把解析结果写入到最终的输出文件中.
int main() 
{
    // 1. 进行枚举路径
    vector<string> file_list;
    bool ret = EnumFile(g_input_path, &file_list);
    if (!ret) 
    {
        std::cout << "枚举路径失败!" << std::endl;
        return 1;
    }
    // 2. 遍历枚举出来的路径, 针对每个文件, 单独进行处理
    std::ofstream output_file(g_output_path.c_str());
    if (!output_file.is_open()) 
    {
        std::cout << "打开output文件失败" << std::endl;
        return 1;
    }
    for (const auto &file_path : file_list) 
    {
        std::cout << file_path << std::endl;
        // 先创建一个 DocInfo 对象
        DocInfo doc_info;
        // 通过一个函数来负责这里的解析工作
        ret = ParseFile(file_path, &doc_info);
        if (!ret) 
        {
            std::cout << "解析该文件失败: " << file_path << std::endl;
            continue;
        }
        // 3. 把解析出来的结果写入到最终的输出文件中.
        WriteOutput(doc_info, output_file);
    }
    output_file.close();
    return 0;
}
