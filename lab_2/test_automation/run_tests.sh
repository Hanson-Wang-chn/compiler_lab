#!/bin/bash
# ============================================================
# LL(1)语法分析器自动化测试脚本
# 功能：批量执行测试用例，比对实际输出与预期输出
# 使用方法：./run_tests.sh
# ============================================================

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # 无颜色

# 测试目录
TEST_DIR="./testcases"
EXPECTED_DIR="./expected"
OUTPUT_DIR="./output"
PARSER="./parser"

# 统计变量
TOTAL=0
PASSED=0
FAILED=0

# 检查解析器是否存在
check_parser() {
    if [ ! -f "$PARSER" ]; then
        echo -e "${YELLOW}解析器不存在，正在编译...${NC}"
        g++ -std=c++11 -o parser main.cpp
        if [ $? -ne 0 ]; then
            echo -e "${RED}编译失败！${NC}"
            exit 1
        fi
        echo -e "${GREEN}编译成功！${NC}"
    fi
}

# 创建必要的目录
setup_dirs() {
    mkdir -p "$TEST_DIR" "$EXPECTED_DIR" "$OUTPUT_DIR"
}

# 运行单个测试
run_test() {
    local test_file=$1
    local test_name=$(basename "$test_file" .txt)
    local expected_file="$EXPECTED_DIR/${test_name}.txt"
    local output_file="$OUTPUT_DIR/${test_name}.txt"

    TOTAL=$((TOTAL + 1))

    # 运行解析器
    cat "$test_file" | "$PARSER" > "$output_file" 2>&1

    # 检查预期输出文件是否存在
    if [ ! -f "$expected_file" ]; then
        echo -e "${YELLOW}[跳过]${NC} $test_name - 缺少预期输出文件"
        return
    fi

    # 比较输出
    if diff -q "$output_file" "$expected_file" > /dev/null 2>&1; then
        echo -e "${GREEN}[通过]${NC} $test_name"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}[失败]${NC} $test_name"
        FAILED=$((FAILED + 1))
        echo "  差异："
        diff "$output_file" "$expected_file" | head -10
    fi
}

# 运行所有测试
run_all_tests() {
    echo "============================================"
    echo "       LL(1)语法分析器自动化测试"
    echo "============================================"
    echo ""

    # 遍历所有测试文件
    for test_file in "$TEST_DIR"/*.txt; do
        if [ -f "$test_file" ]; then
            run_test "$test_file"
        fi
    done

    echo ""
    echo "============================================"
    echo "测试结果统计"
    echo "============================================"
    echo "总计: $TOTAL"
    echo -e "通过: ${GREEN}$PASSED${NC}"
    echo -e "失败: ${RED}$FAILED${NC}"

    if [ $FAILED -eq 0 ] && [ $TOTAL -gt 0 ]; then
        echo -e "${GREEN}所有测试通过！${NC}"
    elif [ $TOTAL -eq 0 ]; then
        echo -e "${YELLOW}未找到测试用例${NC}"
    fi
}

# 主函数
main() {
    setup_dirs
    check_parser
    run_all_tests
}

main
