#!/bin/bash
# LR(1)语法分析器自动化测试脚本
# 用于批量测试输入文件并验证输出结果

# 颜色定义，用于美化输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 测试目录和可执行文件配置
TEST_DIR="./testcases"
EXPECTED_DIR="./expected"
OUTPUT_DIR="./output"
EXECUTABLE="../test_parser"

# 统计变量
TOTAL=0
PASSED=0
FAILED=0

# 显示脚本使用说明
show_usage() {
    echo "用法: $0 [选项]"
    echo "选项:"
    echo "  -h, --help      显示帮助信息"
    echo "  -c, --clean     清理输出目录"
    echo "  -v, --verbose   显示详细输出"
    echo "  -g, --generate  生成测试用例模板"
}

# 清理输出目录
clean_output() {
    echo "清理输出目录..."
    rm -rf "$OUTPUT_DIR"
    mkdir -p "$OUTPUT_DIR"
    echo "清理完成"
}

# 生成测试用例模板
generate_template() {
    mkdir -p "$TEST_DIR" "$EXPECTED_DIR"

    # 创建示例测试用例
    cat > "$TEST_DIR/test_simple.txt" << 'EOF'
{
ID = NUM ;
}
EOF

    cat > "$EXPECTED_DIR/test_simple.txt" << 'EOF'
program =>
compoundstmt =>
{ stmts } =>
{ stmt stmts } =>
{ stmt } =>
{ assgstmt } =>
{ ID = arithexpr ; } =>
{ ID = multexpr arithexprprime ; } =>
{ ID = multexpr ; } =>
{ ID = simpleexpr multexprprime ; } =>
{ ID = simpleexpr ; } =>
{ ID = NUM ; }
EOF

    echo "测试用例模板已生成到 $TEST_DIR 和 $EXPECTED_DIR"
}

# 运行单个测试
run_test() {
    local input_file=$1
    local basename=$(basename "$input_file" .txt)
    local expected_file="$EXPECTED_DIR/${basename}.txt"
    local output_file="$OUTPUT_DIR/${basename}.txt"

    TOTAL=$((TOTAL + 1))

    # 检查可执行文件是否存在
    if [ ! -f "$EXECUTABLE" ]; then
        echo -e "${RED}[错误]${NC} 可执行文件 $EXECUTABLE 不存在，请先编译"
        return 1
    fi

    # 运行分析器
    "$EXECUTABLE" < "$input_file" > "$output_file" 2>&1

    # 检查预期输出文件是否存在
    if [ ! -f "$expected_file" ]; then
        echo -e "${YELLOW}[跳过]${NC} $basename - 无预期输出文件"
        return 0
    fi

    # 比较输出结果
    if diff -q "$output_file" "$expected_file" > /dev/null 2>&1; then
        echo -e "${GREEN}[通过]${NC} $basename"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}[失败]${NC} $basename"
        FAILED=$((FAILED + 1))

        if [ "$VERBOSE" = "1" ]; then
            echo "--- 差异 ---"
            diff "$output_file" "$expected_file"
            echo "------------"
        fi
    fi
}

# 主测试函数
run_all_tests() {
    echo "=========================================="
    echo "     LR(1)语法分析器自动化测试"
    echo "=========================================="
    echo ""

    # 确保输出目录存在
    mkdir -p "$OUTPUT_DIR"

    # 检查测试目录
    if [ ! -d "$TEST_DIR" ]; then
        echo -e "${RED}[错误]${NC} 测试目录 $TEST_DIR 不存在"
        echo "请使用 -g 选项生成测试用例模板"
        exit 1
    fi

    # 遍历所有测试文件
    for input_file in "$TEST_DIR"/*.txt; do
        if [ -f "$input_file" ]; then
            run_test "$input_file"
        fi
    done

    # 输出统计结果
    echo ""
    echo "=========================================="
    echo "              测试统计"
    echo "=========================================="
    echo "总计: $TOTAL"
    echo -e "通过: ${GREEN}$PASSED${NC}"
    echo -e "失败: ${RED}$FAILED${NC}"
    echo ""

    if [ $FAILED -eq 0 ] && [ $TOTAL -gt 0 ]; then
        echo -e "${GREEN}所有测试通过！${NC}"
        exit 0
    elif [ $TOTAL -eq 0 ]; then
        echo -e "${YELLOW}没有找到测试用例${NC}"
        exit 0
    else
        echo -e "${RED}存在失败的测试${NC}"
        exit 1
    fi
}

# 解析命令行参数
VERBOSE=0
while [ "$#" -gt 0 ]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -c|--clean)
            clean_output
            exit 0
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -g|--generate)
            generate_template
            exit 0
            ;;
        *)
            echo "未知选项: $1"
            show_usage
            exit 1
            ;;
    esac
done

# 运行测试
run_all_tests
