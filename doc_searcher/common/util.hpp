#pragma once

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/algorithm/string.hpp>

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using std::vector;

namespace common 
{

class Util 
{
  public:
    // 负责从指定的路径中, 读取出文件的整体内容, 读到 output 这个 string 里
    static bool Read(const string &input_path, string *output) 
    {
        std::ifstream file(input_path.c_str());
        if (!file.is_open()) 
        {
            return false;
        }
        // 按行读取整个文件的内容将读到的每行结果追加到output中.
        // getline 功能就是读取文件中的一行.
        // 如果读取成功, 就把内容放到了 line 中. 并返回 true
        // 如果读取失败(读到文件末尾), 返回 false
        string line;
        while (std::getline(file, line)) 
        {
            *output += (line + "\n");
        }
        file.close();
        return true;
    }

    // 基于 boost 中的字符串切分, 封装一下
    // delimiter 表示分割符, 按照啥字符来切分.
    // token_compress_off:
    // 例如有个字符串: aaa\3bbb\3\3ccc
    // 此时按照 \3 进行切分,
    // 切分结果可能有两种风格:
    // 1. 结果有三个部分,  aaa bbb ccc       token_compress_on 有分割符相邻时,
    // 会压缩切分结果
    // 2. 结果有四个部分,  aaa bbb "" ccc    token_compress_off
    // 不会压缩切分结果的.
    static void Split(const string &input, const string &delimiter,vector<string> *output) 
    {
        boost::split(*output, input, boost::is_any_of(delimiter),boost::token_compress_off);
    }
};

} // namespace common
