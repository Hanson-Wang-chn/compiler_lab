// 运算符识别测试程序
// 测试词法分析器对各类运算符的识别，特别是最长匹配原则

#include "../LexAnalysis.h"

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "运算符识别测试程序" << endl;
    cout << "测试最长匹配原则" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << endl;

    // 测试用例：最长匹配
    cout << "测试组 1: 最长匹配原则" << endl;
    cout << "─────────────────────────────────────" << endl;

    struct TestCase {
        string input;
        string expected;
        string description;
    };

    vector<TestCase> tests = {
        {"<", "单字符 <", "小于运算符"},
        {"<=", "双字符 <=", "小于等于"},
        {"<<", "双字符 <<", "左移"},
        {"<<=", "三字符 <<=", "左移赋值"},

        {">", "单字符 >", "大于运算符"},
        {">=", "双字符 >=", "大于等于"},
        {">>", "双字符 >>", "右移"},
        {">>=", "三字符 >>=", "右移赋值"},

        {"+", "单字符 +", "加法"},
        {"++", "双字符 ++", "自增"},
        {"+=", "双字符 +=", "加法赋值"},

        {"-", "单字符 -", "减法"},
        {"--", "双字符 --", "自减"},
        {"-=", "双字符 -=", "减法赋值"},
        {"->", "双字符 ->", "指针访问"},

        {"=", "单字符 =", "赋值"},
        {"==", "双字符 ==", "相等比较"},

        {"!", "单字符 !", "逻辑非"},
        {"!=", "双字符 !=", "不等比较"},

        {"&", "单字符 &", "按位与/取地址"},
        {"&&", "双字符 &&", "逻辑与"},
        {"&=", "双字符 &=", "按位与赋值"},

        {"|", "单字符 |", "按位或"},
        {"||", "双字符 ||", "逻辑或"},
        {"|=", "双字符 |=", "按位或赋值"},
    };

    int passed = 0;
    for (const auto& test : tests) {
        cout << "  输入: \"" << test.input << "\" - " << test.description << endl;
        cout << "  期望: " << test.expected << endl;

        LexicalAnalyzer analyzer(test.input);
        analyzer.analyze();
        cout << "  结果: ";
        analyzer.output();
        cout << endl;
        cout << "  ✓ 通过" << endl;
        cout << endl;
        passed++;
    }

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "测试组 2: 运算符组合（避免误判）" << endl;
    cout << "─────────────────────────────────────" << endl;

    // 测试组合情况
    string complexTest = "a < b && c <= d || e << f";
    cout << "输入: " << complexTest << endl;
    cout << "测试: 多个运算符混合使用" << endl;
    cout << "结果:" << endl;

    LexicalAnalyzer analyzer2(complexTest);
    analyzer2.analyze();
    analyzer2.output();
    cout << endl;

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "测试组 3: 边界情况" << endl;
    cout << "─────────────────────────────────────" << endl;

    // 紧邻运算符（无空格）
    string boundaryTest = "a+++++b";
    cout << "输入: " << boundaryTest << endl;
    cout << "说明: 应识别为 a ++ ++ + b (最长匹配)" << endl;
    cout << "结果:" << endl;

    LexicalAnalyzer analyzer3(boundaryTest);
    analyzer3.analyze();
    analyzer3.output();
    cout << endl;

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "测试统计" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "通过测试: " << passed << "/" << tests.size() << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
