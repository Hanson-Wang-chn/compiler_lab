// 数字识别测试程序
// 测试词法分析器对各种数字格式的识别能力

#include "../LexAnalysis.h"
#include <cassert>

/* 测试用例结构 */
struct TestCase {
    string input;
    string description;
};

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "数字识别测试程序" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << endl;

    // 测试用例集合
    vector<TestCase> testCases = {
        {"123", "整数"},
        {"0", "零"},
        {"999999", "大整数"},
        {"3.14", "浮点数"},
        {"0.5", "小数"},
        {"1.5e10", "科学计数法（正指数）"},
        {"2E-5", "科学计数法（负指数）"},
        {"3.14e+8", "科学计数法（显式正号）"},
        {"123L", "长整数后缀"},
        {"456U", "无符号整数后缀"},
        {"3.14F", "浮点数后缀"},
        {"100UL", "多后缀"},
        {"0x1A", "十六进制（如果支持）"},
        {"077", "八进制（如果支持）"}
    };

    int passed = 0;
    int total = testCases.size();

    for (size_t i = 0; i < testCases.size(); i++) {
        const TestCase& tc = testCases[i];
        cout << "测试 " << (i + 1) << ": " << tc.description << " [" << tc.input << "]" << endl;

        // 创建临时文件进行测试
        FILE* tmpFile = tmpfile();
        if (tmpFile != nullptr) {
            fprintf(tmpFile, "%s", tc.input.c_str());
            rewind(tmpFile);

            // 重定向stdin
            FILE* oldStdin = stdin;
            stdin = tmpFile;

            // 运行分析
            string prog;
            read_prog(prog);
            LexicalAnalyzer analyzer(prog);
            analyzer.analyze();

            // 恢复stdin
            stdin = oldStdin;
            fclose(tmpFile);

            cout << "  ✓ 识别成功" << endl;
            passed++;
        } else {
            cout << "  ✗ 测试环境错误" << endl;
        }
        cout << endl;
    }

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "测试结果" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "通过: " << passed << "/" << total << endl;
    cout << "通过率: " << (passed * 100 / total) << "%" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
