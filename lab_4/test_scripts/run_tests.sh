#!/bin/bash
# 自动化测试脚本
# 用于批量测试Translation Schema的正确性

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# 项目根目录
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# 编译程序
echo "正在编译程序..."
cd "$PROJECT_DIR"
g++ -std=c++11 -o test_runner main.cpp 2>&1

if [ $? -ne 0 ]; then
    echo -e "${RED}编译失败！${NC}"
    exit 1
fi
echo -e "${GREEN}编译成功！${NC}"
echo ""

# 统计变量
TOTAL=0
PASSED=0
FAILED=0

# 遍历所有测试用例
echo "开始运行测试用例..."
echo "========================================"

for input_file in "$SCRIPT_DIR"/test*.in; do
    # 检查是否存在测试文件
    if [ ! -f "$input_file" ]; then
        echo "没有找到测试用例文件"
        exit 0
    fi

    # 获取测试用例编号
    test_name=$(basename "$input_file" .in)
    expected_file="$SCRIPT_DIR/${test_name}.out"

    # 检查期望输出文件是否存在
    if [ ! -f "$expected_file" ]; then
        echo -e "${YELLOW}警告: ${test_name} 缺少期望输出文件，跳过${NC}"
        continue
    fi

    TOTAL=$((TOTAL + 1))

    # 运行测试
    actual_output=$("$PROJECT_DIR/test_runner" < "$input_file" 2>&1)
    expected_output=$(cat "$expected_file")

    # 比较结果
    if [ "$actual_output" = "$expected_output" ]; then
        echo -e "${GREEN}✓ ${test_name} 通过${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}✗ ${test_name} 失败${NC}"
        echo "  期望输出:"
        echo "$expected_output" | sed 's/^/    /'
        echo "  实际输出:"
        echo "$actual_output" | sed 's/^/    /'
        FAILED=$((FAILED + 1))
    fi
done

echo "========================================"
echo ""

# 输出统计结果
echo "测试结果统计:"
echo "  总计: $TOTAL"
echo -e "  通过: ${GREEN}$PASSED${NC}"
echo -e "  失败: ${RED}$FAILED${NC}"

if [ $FAILED -eq 0 ] && [ $TOTAL -gt 0 ]; then
    echo ""
    echo -e "${GREEN}所有测试用例通过！${NC}"
    exit 0
else
    exit 1
fi
