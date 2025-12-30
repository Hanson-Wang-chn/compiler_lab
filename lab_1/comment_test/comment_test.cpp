// 注释处理测试程序
// 测试词法分析器对块注释和行注释的处理能力

#include "../LexAnalysis.h"

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "注释处理测试程序" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << endl;

    // 测试用例1：块注释
    cout << "测试 1: 块注释" << endl;
    cout << "输入: /* 这是一个块注释 */ int x;" << endl;
    cout << "期望: 注释应被识别为编号79" << endl;
    cout << endl;

    // 测试用例2：行注释
    cout << "测试 2: 行注释" << endl;
    cout << "输入: // 这是一个行注释\\nint y;" << endl;
    cout << "期望: 注释应被识别为编号79" << endl;
    cout << endl;

    // 测试用例3：嵌套代码中的注释
    cout << "测试 3: 多个注释" << endl;
    cout << "输入: /* 注释1 */ int a; // 注释2\\n/* 注释3 */" << endl;
    cout << "期望: 三个注释都被识别" << endl;
    cout << endl;

    // 测试用例4：注释中的特殊字符
    cout << "测试 4: 注释中的特殊字符" << endl;
    cout << "输入: /* 包含特殊符号：!@#$%^&*() */" << endl;
    cout << "期望: 正确识别，不误判为运算符" << endl;
    cout << endl;

    // 测试用例5：未闭合的注释（鲁棒性测试）
    cout << "测试 5: 边界情况 - 注释紧邻代码" << endl;
    cout << "输入: int/*注释*/x;" << endl;
    cout << "期望: 正确分离注释和代码" << endl;
    cout << endl;

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "运行完整测试..." << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << endl;

    // 综合测试
    string testInput =
        "/* 这是块注释 */\n"
        "// 这是行注释\n"
        "int main() {\n"
        "    /* 多行\n"
        "       块注释 */\n"
        "    int x = 10; // 行尾注释\n"
        "    return 0;\n"
        "}\n";

    // 模拟分析
    LexicalAnalyzer analyzer(testInput);
    analyzer.analyze();
    cout << "分析结果：" << endl;
    analyzer.output();
    cout << endl;

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "测试完成" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
