// 鲁棒性测试程序
// 测试词法分析器对边界情况和异常输入的处理能力

#include "../LexAnalysis.h"

void testCase(const string& name, const string& input, const string& description) {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "测试: " << name << endl;
    cout << "说明: " << description << endl;
    cout << "输入: ";
    if (input.empty()) {
        cout << "(空输入)" << endl;
    } else if (input.length() > 50) {
        cout << input.substr(0, 47) << "..." << endl;
    } else {
        cout << input << endl;
    }
    cout << "输出:" << endl;

    try {
        LexicalAnalyzer analyzer(input);
        analyzer.analyze();
        analyzer.output();
        if (input.empty()) {
            cout << "(无输出)";
        }
        cout << endl;
        cout << "状态: ✓ 正常处理" << endl;
    } catch (const exception& e) {
        cout << "状态: ✗ 异常 - " << e.what() << endl;
    } catch (...) {
        cout << "状态: ✗ 未知异常" << endl;
    }
    cout << endl;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "词法分析器鲁棒性测试" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << endl;

    // 测试组1：空输入和空白
    cout << "═══════════════════════════════════════" << endl;
    cout << "测试组 1: 空输入和空白处理" << endl;
    cout << "═══════════════════════════════════════" << endl;
    cout << endl;

    testCase("空输入", "", "完全空的输入");
    testCase("只有空格", "    ", "只包含空格");
    testCase("只有换行", "\n\n\n", "只包含换行符");
    testCase("混合空白", " \t\n\r ", "各种空白字符");

    // 测试组2：未闭合情况
    cout << "═══════════════════════════════════════" << endl;
    cout << "测试组 2: 未闭合情况处理" << endl;
    cout << "═══════════════════════════════════════" << endl;
    cout << endl;

    testCase("未闭合块注释", "/* 这是一个未闭合的注释", "注释没有结束符");
    testCase("未闭合字符串", "\"Hello", "字符串缺少结束引号");
    testCase("部分运算符", "int a = 1 +", "表达式不完整");

    // 测试组3：边界情况
    cout << "═══════════════════════════════════════" << endl;
    cout << "测试组 3: 边界情况" << endl;
    cout << "═══════════════════════════════════════" << endl;
    cout << endl;

    testCase("超长标识符",
             "thisIsAVeryVeryVeryLongIdentifierNameThatExceedsNormalLength_12345",
             "超长变量名");

    testCase("超大数字",
             "999999999999999999999999999999999999",
             "超大整数");

    testCase("紧邻符号",
             "int/**/a/**/=/**/10/**/;",
             "符号间无空格，只有注释");

    testCase("连续运算符",
             "a+++++b",
             "多个加号连续（测试最长匹配）");

    // 测试组4：特殊字符
    cout << "═══════════════════════════════════════" << endl;
    cout << "测试组 4: 特殊字符处理" << endl;
    cout << "═══════════════════════════════════════" << endl;
    cout << endl;

    testCase("中文字符",
             "int 变量 = 10;",
             "包含中文（非法字符）");

    testCase("特殊符号",
             "int a @ = 10;",
             "包含非法符号@");

    testCase("制表符分隔",
             "int\ta\t=\t10;",
             "使用制表符分隔");

    // 测试组5：嵌套和复杂情况
    cout << "═══════════════════════════════════════" << endl;
    cout << "测试组 5: 复杂嵌套" << endl;
    cout << "═══════════════════════════════════════" << endl;
    cout << endl;

    testCase("注释中的引号",
             "/* 这里有\"引号\" */ int a;",
             "注释内包含引号");

    testCase("字符串中的注释符",
             "char* s = \"/* not a comment */\";",
             "字符串内包含注释符号");

    testCase("转义字符",
             "char c = '\\n';",
             "转义字符处理");

    // 测试组6：极端情况
    cout << "═══════════════════════════════════════" << endl;
    cout << "测试组 6: 极端情况" << endl;
    cout << "═══════════════════════════════════════" << endl;
    cout << endl;

    testCase("只有注释",
             "/* comment1 */ // comment2",
             "整个文件只有注释");

    testCase("只有一个字符",
             ";",
             "最小有效输入");

    testCase("所有关键字",
             "auto break case char const continue default do "
             "double else enum extern float for goto if int long "
             "register return short signed sizeof static struct "
             "switch typedef union unsigned void volatile while",
             "包含所有32个关键字");

    // 测试统计
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "鲁棒性测试完成" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << endl;
    cout << "总结：" << endl;
    cout << "- 词法分析器应能处理所有边界情况而不崩溃" << endl;
    cout << "- 对于非法输入，应优雅降级（跳过或忽略）" << endl;
    cout << "- 确保鲁棒性是高质量编译器的基础" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
