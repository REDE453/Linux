#include "searcher.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <fstream>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <string>
#include <algorithm>
#include <jsoncpp/json/json.h>

#include "../common/util.hpp"

namespace searcher {

//////////////////////////////////////////////////////////////////////////////////////////////////////
// 以下代码为 Index 模块的代码
//////////////////////////////////////////////////////////////////////////////////////////////////////

const char* const DICT_PATH = "../jieba_dict/jieba.dict.utf8";
const char* const HMM_PATH = "../jieba_dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "../jieba_dict/user.dict.utf8";
const char* const IDF_PATH = "../jieba_dict/idf.utf8";
const char* const STOP_WORD_PATH = "../jieba_dict/stop_words.utf8";

Index::Index() : jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH) {
}

const DocInfo* Index::GetDocInfo(int64_t doc_id) {
    if (doc_id < 0 || doc_id >= forward_index.size()) {
        return nullptr;
    }
    return &forward_index[doc_id];
}

const InvertedList* Index::GetInvertedList(const string& key) {
    auto it = inverted_index.find(key);
    if (it == inverted_index.end()) {
        return nullptr;
    }
    return &it->second;
}

// 上面的两个函数实现起来都比较容易. 接下来这个函数(构建索引),
// 实现起来就要更复杂一些.
bool Index::Build(const string& input_path) {
    // 1. 按行读取输入文件内容(上个环节预处理模块生成的 raw_input 文件)
    //    raw_input 的结构: 是一个行文本文件, 每一行对应一个文档.
    //    每一行又分成三个部分, 使用 \3 来切分, 分别是标题, url, 正文
    std::cerr << "开始构建索引" << std::endl;
    std::ifstream file(input_path.c_str());
    if (!file.is_open()) {
        std::cout << "raw_input 文件打开失败" << std::endl;
        return false;
    }
    string line;
    while (std::getline(file, line)) {
        // 2. 针对当前行, 解析成 DocInfo 对象, 并构造为正排索引
        DocInfo* doc_info = BuildForward(line);
        if (doc_info == nullptr) {
            // 当前文档构造正排出现问题.
            std::cout << "构建正排失败!" << std::endl;
            continue;
        }
        // 3. 根据当前的 DocInfo 对象, 进行解析, 构造成倒排索引.
        BuildInverted(*doc_info);

        // 此处直接加循环不好. 可能会影响到整个程序的执行效率
        // 既需要能看到进度, 又要尽量小的影响到原来的程序执行.
        if (doc_info->doc_id % 100 == 0) {
            std::cerr << doc_info->doc_id << std::endl;
        }
    }
    std::cerr << "结束构建索引" << std::endl;
    file.close();
    return true;
}

// 核心操作: 按照 \3 对 line 进行切分, 第一个部分就是标题, 第二个部分就是 url,
// 第三个部分就是正文
// C++ 标准库中, 也没有字符串切分的操作.
DocInfo* Index::BuildForward(const string& line) {
    // 1. 先把 line 按照 \3 切分成 3 个部分
    vector<string> tokens;
    common::Util::Split(line, "\3", &tokens);
    if (tokens.size() != 3) {
        // 如果切分结果不是 3 份, 就认为当前这一行是存在问题的,
        // 认为该文档构造失败.
        return nullptr;
    }
    // 2. 把切分结果填充到 DocInfo 对象中
    DocInfo doc_info;
    doc_info.doc_id = forward_index.size();
    doc_info.title = tokens[0];
    doc_info.url = tokens[1];
    doc_info.content = tokens[2];
    forward_index.push_back(std::move(doc_info));
    // 3. 返回结果
    // 注意体会这里可能存在的野指针问题. C++ 中的经典错误,
    // 也是面试中的重要考点!!! return &doc_info;
    return &forward_index.back();
}

// 倒排是一个 hash 表.
// key 是词 (针对文档的分词结果)
// value 是倒排拉链 (包含若干个 Weight 对象)
// 每次遍历到一个文档, 就要分析这个文档, 并且把相关信息, 更新到倒排结构中.
void Index::BuildInverted(const DocInfo& doc_info) {
    // 0. 创建专门用于统计词频的结构
    struct WordCnt {
        int title_cnt;
        int content_cnt;
        WordCnt() : title_cnt(0), content_cnt(0) {
        }
    };
    unordered_map<string, WordCnt> word_cnt_map;
    // 1. 针对标题进行分词
    vector<string> title_token;
    CutWord(doc_info.title, &title_token);
    // 2. 遍历分词结果, 统计每个词出现的次数
    for (string word : title_token) {
        // map/unorder_map [ ] 有两个功能. key 不存在就添加, 存在就修改.
        // 注意, 此处需要考虑 大小写 的问题.
        // 针对 HELLO, hello, Hello, 算成三个单词各自出现一次,
        // 还是一个单词出现三次 [合理做法]
        // 在统计之前先把单词统一转成全小写
        // C++ 标准库中没有转小写的操作~~ , 使用 boost 来完成
        // to_lower 会直接修改当前单词. 而 word 现在是一个 const 引用.
        boost::to_lower(word);
        ++word_cnt_map[word].title_cnt;
    }
    // 3. 针对正文进行分词
    vector<string> content_token;
    CutWord(doc_info.content, &content_token);
    // 4. 遍历分词结果, 统计每个词出现的次数
    for (string word : content_token) {
        boost::to_lower(word);
        ++word_cnt_map[word].content_cnt;
    }
    // 5. 根据统计结果, 整合出 Weight 对象, 并把结果更新到 倒排索引 中即可
    //    word_pair 每次循环就是对应到 map 中的一个键值对(std::pair)
    for (const auto& word_pair : word_cnt_map) {
        // 构造 Weight 对象
        Weight weight;
        weight.doc_id = doc_info.doc_id;
        // 权重 = 标题出现次数 * 10 + 正文出现次数
        weight.weight = 10 * word_pair.second.title_cnt + word_pair.second.content_cnt;
        weight.word = word_pair.first;

        // 把 weight 对象插入到倒排索引中. 需要先找到对应的倒排拉链,
        // 然后追加到拉链末尾即可.
        InvertedList& inverted_list = inverted_index[word_pair.first];
        inverted_list.push_back(std::move(weight));
    }
}

void Index::CutWord(const string& input, vector<string>* output) {
    jieba.CutForSearch(input, *output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// 以下代码为 Searcher 模块的代码
//////////////////////////////////////////////////////////////////////////////////////////////////////

bool Searcher::Init(const string& input_path) {
    return index->Build(input_path);
}

// 这个函数进行的操作就是把查询词进行搜索, 得到搜索结果.
bool Searcher::Search(const string& query, string* output) {
    // 1. [分词] 针对查询词进行分词
    vector<string> tokens;
    index->CutWord(query, &tokens);
    // 2. [触发] 根据分词结果, 查倒排, 把相关的文档都获取到
    vector<Weight> all_token_result;
    for (string word : tokens) {
        // 别忘了, 在做索引的时候, 已经把其中的词忽略了大小写(统一转成小写)
        // 查倒排索引的时候, 也需要把查询词统一转成小写.
        boost::to_lower(word);
        auto* inverted_list = index->GetInvertedList(word);
        if (inverted_list == nullptr) {
            // 该词在倒排索引中不存在. 如果这个词比较生僻, 在所有的文档
            // 中都没有出现过, 此时得到的倒排拉链就是 nullptr
            continue;
        }
        // tokens 包含多个结果, 需要把多个结果合并到一起, 才能进行统一的排序
        all_token_result.insert(all_token_result.end(), inverted_list->begin(), inverted_list->end());
    }
    // 3. [排序] 把刚才查到的这些文档的倒排拉链合并到一起并按照权重进行降序排序.
    std::sort(all_token_result.begin(), all_token_result.end(), [](const Weight& w1, const Weight& w2) {
        // 如果要实现升序排序, 就写成 w1.weight < w2.weight
        // 如果要实现降序排序, 就写成 w1.weight > w2.weight
        return w1.weight > w2.weight;
    });

    // 4. [包装结果] 把得到的这些倒排拉链中的 文档 id 获取到, 然后去查正排.
    //    再把 doc_info 中的内容构造成最终预期的格式.(JSON)
    //    使用 jsoncpp 这个库来实现 json 的操作.
    Json::Value results; // 这个 results 中包含了若干个搜索结果. 每个搜索结果就是一个 JSON 对象
    for (const auto& weight : all_token_result) {
        // 根据 weight 中的 doc_id 查正排
        const DocInfo* doc_info = index->GetDocInfo(weight.doc_id);
        // 把这个 doc_info 对象再进一步的包装成一个 JSON 对象
        Json::Value result;
        result["title"] = doc_info->title;
        result["url"] = doc_info->url;
        result["desc"] = GenerateDesc(doc_info->content, weight.word);
        results.append(result);
    }
    // 最后一步, 把得到的 results 这个 JSON 对象序列化成字符串. 写入 output 中
    Json::FastWriter writer;
    *output = writer.write(results);
    return true;
}

string Searcher::GenerateDesc(const string& content, const string& word) {
    // 根据正文, 找到 word 出现的位置.
    // 以该位置为中心, 往前找 60 个字节, 作为描述的起始位置.
    // 再从起始位置开始往后找 160 个字节, 作为整个描述.
    // 需要注意边界条件.
    // 例如, 如果前面不够 60 个字节了, 就从 0 开始.
    // 例如, 后面的内容不够了, 就到末尾结束.
    // 如果后面的内容显示不下, 可以使用 ... 省略号来表示.
    // [注意!] 60/160 这俩数字都是拍脑门出来的. 实际写的时候都可以灵活调整.

    // 1. 先找到 word 在正文中出现的位置.
    size_t first_pos = content.find(word);
    size_t desc_beg = 0;
    if (first_pos == string::npos) {
        // 该词在正文中不存在. (例如该词只出现在标题中, 没在正文中出现).
        // 如果找不到, 就直接从头开始作为起始位置.
        if (content.size() < 160) {
            return content;
        }
        string desc = content.substr(0, 160);
        desc[desc.size() - 1] = '.';
        desc[desc.size() - 2] = '.';
        desc[desc.size() - 3] = '.';
        return desc;
    }
    // 2. 找到了 first_pos 位置, 以这个位置为基准, 往前找一些字节.
    desc_beg = first_pos < 60 ? 0 : first_pos - 60;
    if (desc_beg + 160 >= content.size()) {
        // desc_beg 后面的内容不够 160 了, 直接到末尾结束即可
        return content.substr(desc_beg);
    } else {
        string desc = content.substr(desc_beg, 160);
        desc[desc.size() - 1] = '.';
        desc[desc.size() - 2] = '.';
        desc[desc.size() - 3] = '.';
        return desc;
    }
}

} // namespace searcher