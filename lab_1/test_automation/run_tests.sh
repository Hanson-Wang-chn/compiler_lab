#!/bin/bash

# 自动化测试脚本
# 用于批量测试词法分析器

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "词法分析器自动化测试系统"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# 编译测试程序
echo "正在编译测试程序..."
g++ -std=c++11 -o test_runner test_runner.cpp
if [ $? -ne 0 ]; then
    echo "编译失败！"
    exit 1
fi
echo "编译成功！"
echo ""

# 初始化计数器
total=0
passed=0
failed=0

# 遍历测试用例
for test_file in test_cases/*.c; do
    if [ ! -f "$test_file" ]; then
        continue
    fi

    total=$((total + 1))
    test_name=$(basename "$test_file" .c)
    expected_file="expected_outputs/${test_name}.txt"

    echo -n "测试用例 ${total}: ${test_name} ... "

    # 运行测试
    ./test_runner < "$test_file" > output.tmp 2>&1

    # 比较输出
    if [ -f "$expected_file" ]; then
        if diff -q output.tmp "$expected_file" > /dev/null 2>&1; then
            echo "✓ 通过"
            passed=$((passed + 1))
        else
            echo "✗ 失败"
            failed=$((failed + 1))
            echo "  期望输出: $expected_file"
            echo "  实际输出: output.tmp"
            echo "  差异:"
            diff output.tmp "$expected_file" | head -10
            echo ""
        fi
    else
        echo "⚠ 缺少期望输出文件"
        # 保存输出供参考
        cp output.tmp "expected_outputs/${test_name}.txt.generated"
    fi
done

# 清理临时文件
rm -f output.tmp

# 输出统计信息
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "测试结果统计"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "总测试数: ${total}"
echo "通过: ${passed}"
echo "失败: ${failed}"

if [ $total -gt 0 ]; then
    pass_rate=$((passed * 100 / total))
    echo "通过率: ${pass_rate}%"
fi
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# 返回状态码
if [ $failed -eq 0 ] && [ $total -gt 0 ]; then
    echo "所有测试通过！"
    exit 0
else
    exit 1
fi
