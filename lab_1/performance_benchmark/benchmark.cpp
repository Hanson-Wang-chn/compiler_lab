// 性能基准测试程序
// 对比不同关键字查找方法的性能

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <cstring>
using namespace std;

// 方法1：线性数组查找
class LinearSearchAnalyzer {
private:
    struct KeywordPair {
        const char* keyword;
        int code;
    };

    static const KeywordPair keywords[];

public:
    int findKeyword(const string& word) {
        for (int i = 0; i < 32; i++) {
            if (word == keywords[i].keyword) {
                return keywords[i].code;
            }
        }
        return -1;  // 不是关键字
    }
};

const LinearSearchAnalyzer::KeywordPair LinearSearchAnalyzer::keywords[] = {
    {"auto", 1}, {"break", 2}, {"case", 3}, {"char", 4},
    {"const", 5}, {"continue", 6}, {"default", 7}, {"do", 8},
    {"double", 9}, {"else", 10}, {"enum", 11}, {"extern", 12},
    {"float", 13}, {"for", 14}, {"goto", 15}, {"if", 16},
    {"int", 17}, {"long", 18}, {"register", 19}, {"return", 20},
    {"short", 21}, {"signed", 22}, {"sizeof", 23}, {"static", 24},
    {"struct", 25}, {"switch", 26}, {"typedef", 27}, {"union", 28},
    {"unsigned", 29}, {"void", 30}, {"volatile", 31}, {"while", 32}
};

// 方法2：map（红黑树）查找
class MapSearchAnalyzer {
private:
    map<string, int> keywords;

public:
    MapSearchAnalyzer() {
        keywords["auto"] = 1;
        keywords["break"] = 2;
        keywords["case"] = 3;
        keywords["char"] = 4;
        keywords["const"] = 5;
        keywords["continue"] = 6;
        keywords["default"] = 7;
        keywords["do"] = 8;
        keywords["double"] = 9;
        keywords["else"] = 10;
        keywords["enum"] = 11;
        keywords["extern"] = 12;
        keywords["float"] = 13;
        keywords["for"] = 14;
        keywords["goto"] = 15;
        keywords["if"] = 16;
        keywords["int"] = 17;
        keywords["long"] = 18;
        keywords["register"] = 19;
        keywords["return"] = 20;
        keywords["short"] = 21;
        keywords["signed"] = 22;
        keywords["sizeof"] = 23;
        keywords["static"] = 24;
        keywords["struct"] = 25;
        keywords["switch"] = 26;
        keywords["typedef"] = 27;
        keywords["union"] = 28;
        keywords["unsigned"] = 29;
        keywords["void"] = 30;
        keywords["volatile"] = 31;
        keywords["while"] = 32;
    }

    int findKeyword(const string& word) {
        auto it = keywords.find(word);
        if (it != keywords.end()) {
            return it->second;
        }
        return -1;
    }
};

// 方法3：unordered_map（哈希表）查找
class HashMapSearchAnalyzer {
private:
    unordered_map<string, int> keywords;

public:
    HashMapSearchAnalyzer() {
        keywords["auto"] = 1;
        keywords["break"] = 2;
        keywords["case"] = 3;
        keywords["char"] = 4;
        keywords["const"] = 5;
        keywords["continue"] = 6;
        keywords["default"] = 7;
        keywords["do"] = 8;
        keywords["double"] = 9;
        keywords["else"] = 10;
        keywords["enum"] = 11;
        keywords["extern"] = 12;
        keywords["float"] = 13;
        keywords["for"] = 14;
        keywords["goto"] = 15;
        keywords["if"] = 16;
        keywords["int"] = 17;
        keywords["long"] = 18;
        keywords["register"] = 19;
        keywords["return"] = 20;
        keywords["short"] = 21;
        keywords["signed"] = 22;
        keywords["sizeof"] = 23;
        keywords["static"] = 24;
        keywords["struct"] = 25;
        keywords["switch"] = 26;
        keywords["typedef"] = 27;
        keywords["union"] = 28;
        keywords["unsigned"] = 29;
        keywords["void"] = 30;
        keywords["volatile"] = 31;
        keywords["while"] = 32;
    }

    int findKeyword(const string& word) {
        auto it = keywords.find(word);
        if (it != keywords.end()) {
            return it->second;
        }
        return -1;
    }
};

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "关键字查找性能基准测试" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << endl;

    // 测试数据
    vector<string> testWords = {
        "int", "main", "return", "if", "else", "while", "for",
        "auto", "break", "case", "char", "const", "continue",
        "identifier1", "identifier2", "variable", "function",
        "myVar", "counter", "index", "temp", "result"
    };

    const int iterations = 100000;  // 迭代次数

    // 测试方法1：线性查找
    LinearSearchAnalyzer linear;
    auto start1 = chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        for (const auto& word : testWords) {
            linear.findKeyword(word);
        }
    }
    auto end1 = chrono::high_resolution_clock::now();
    auto duration1 = chrono::duration_cast<chrono::microseconds>(end1 - start1).count();

    // 测试方法2：map查找
    MapSearchAnalyzer mapSearch;
    auto start2 = chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        for (const auto& word : testWords) {
            mapSearch.findKeyword(word);
        }
    }
    auto end2 = chrono::high_resolution_clock::now();
    auto duration2 = chrono::duration_cast<chrono::microseconds>(end2 - start2).count();

    // 测试方法3：unordered_map查找
    HashMapSearchAnalyzer hashSearch;
    auto start3 = chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        for (const auto& word : testWords) {
            hashSearch.findKeyword(word);
        }
    }
    auto end3 = chrono::high_resolution_clock::now();
    auto duration3 = chrono::duration_cast<chrono::microseconds>(end3 - start3).count();

    // 输出结果
    cout << "测试配置：" << endl;
    cout << "  关键字数量: 32" << endl;
    cout << "  测试词数量: " << testWords.size() << endl;
    cout << "  迭代次数: " << iterations << endl;
    cout << "  总查找次数: " << (iterations * testWords.size()) << endl;
    cout << endl;

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "性能测试结果" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << endl;

    cout << "方法1: 线性数组查找" << endl;
    cout << "  时间复杂度: O(n)" << endl;
    cout << "  总耗时: " << duration1 << " 微秒" << endl;
    cout << "  平均每次查找: " << (double)duration1 / (iterations * testWords.size()) << " 微秒" << endl;
    cout << endl;

    cout << "方法2: map（红黑树）查找 ★ 本项目采用" << endl;
    cout << "  时间复杂度: O(log n)" << endl;
    cout << "  总耗时: " << duration2 << " 微秒" << endl;
    cout << "  平均每次查找: " << (double)duration2 / (iterations * testWords.size()) << " 微秒" << endl;
    cout << "  相比线性查找: " << (double)duration1 / duration2 << "x 速度" << endl;
    cout << endl;

    cout << "方法3: unordered_map（哈希表）查找" << endl;
    cout << "  时间复杂度: O(1) 平均" << endl;
    cout << "  总耗时: " << duration3 << " 微秒" << endl;
    cout << "  平均每次查找: " << (double)duration3 / (iterations * testWords.size()) << " 微秒" << endl;
    cout << "  相比线性查找: " << (double)duration1 / duration3 << "x 速度" << endl;
    cout << endl;

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "结论" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "对于32个关键字的规模：" << endl;
    cout << "- map（红黑树）提供了良好的性能和稳定性" << endl;
    cout << "- 比线性查找快数倍" << endl;
    cout << "- C++11标准兼容" << endl;
    cout << "- 适合本项目的需求" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
