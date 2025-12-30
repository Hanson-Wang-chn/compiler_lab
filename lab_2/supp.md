# LL(1)语法分析器 补充文档

本文档对应`innovations.md`中的10条实践亮点，包括详细的说明、代码分析和运行方法。

---

## 目录

1. [面向对象设计模式](#1-面向对象设计模式)
2. [自动化FIRST/FOLLOW集合计算](#2-自动化firstfollow集合计算)
3. [精确行号计算策略](#3-精确行号计算策略)
4. [同步恢复机制](#4-同步恢复机制)
5. [自动化测试脚本](#5-自动化测试脚本)
6. [递归遍历语法树输出](#6-递归遍历语法树输出)
7. [STL嵌套Map实现分析表](#7-stl嵌套map实现分析表)
8. [彩色终端可视化](#8-彩色终端可视化)
9. [Git版本控制](#9-git版本控制)
10. [模板方法设计模式](#10-模板方法设计模式)

---

## 1. 面向对象设计模式

### 设计说明

本项目采用面向对象（OOP）的设计思想，将LL(1)语法分析器划分为以下核心类：

| 类名 | 职责 |
|------|------|
| `Token` | 表示词法单元，包含类型、值、行号 |
| `Lexer` | 词法分析器，将输入字符串分解为Token序列 |
| `Grammar` | 文法定义，管理产生式和FIRST/FOLLOW集合 |
| `ParseTable` | LL(1)分析表，提供产生式查询 |
| `TreeNode` | 语法树节点，支持树形结构和遍历输出 |
| `LLParser` | 语法分析器主类，协调各模块完成解析 |

### 类图结构

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│    Token    │     │   Grammar   │────▶│ ParseTable  │
└─────────────┘     └─────────────┘     └─────────────┘
       ▲                   ▲                   │
       │                   │                   │
┌─────────────┐     ┌─────────────┐           │
│    Lexer    │     │  LLParser   │◀──────────┘
└─────────────┘     └─────────────┘
                           │
                           ▼
                    ┌─────────────┐
                    │  TreeNode   │
                    └─────────────┘
```

### 核心代码示例

```cpp
// Token类：封装词法单元信息
class Token {
public:
    string type;    // token类型
    string value;   // token的实际值
    int line;       // token所在行号

    Token(const string& t = "", const string& v = "", int l = 1)
        : type(t), value(v), line(l) {}
};

// LLParser类：协调各模块完成解析
class LLParser {
private:
    Grammar grammar;           // 文法定义
    ParseTable parseTable;     // 分析表
    vector<Token> tokens;      // Token序列
    // ...
};
```

### 设计优势

1. **高内聚低耦合**：每个类专注于单一职责
2. **易于维护**：修改某一模块不影响其他模块
3. **便于扩展**：可轻松添加新功能（如语义分析）

---

## 2. 自动化FIRST/FOLLOW集合计算

### 算法说明

FIRST集合和FOLLOW集合是构建LL(1)分析表的基础。本项目实现了自动计算这两个集合的算法。

### FIRST集合计算算法

```
FIRST(X)的计算规则：
1. 若X是终结符，则FIRST(X) = {X}
2. 若X是非终结符：
   - 若X -> ε，则将ε加入FIRST(X)
   - 若X -> Y1Y2...Yk，依次处理：
     - 将FIRST(Y1) - {ε}加入FIRST(X)
     - 若ε ∈ FIRST(Y1)，继续处理Y2
     - 若所有Yi都能推导出ε，则将ε加入FIRST(X)
```

### 核心代码

```cpp
// 计算单个符号的FIRST集合（递归实现）
set<string> computeFirst(const string& symbol) {
    set<string> result;

    // 终结符的FIRST就是它本身
    if (terminals.count(symbol)) {
        result.insert(symbol);
        return result;
    }

    // 遍历该非终结符的所有产生式
    for (const auto& prod : productions[symbol]) {
        // 空产生式
        if (prod.size() == 1 && prod[0] == "E") {
            result.insert("E");
            continue;
        }

        // 计算产生式体的FIRST
        bool allHaveEpsilon = true;
        for (const auto& s : prod) {
            set<string> firstOfS = computeFirst(s);

            // 添加除ε以外的所有符号
            for (const auto& f : firstOfS) {
                if (f != "E") result.insert(f);
            }

            // 如果没有ε，停止
            if (firstOfS.find("E") == firstOfS.end()) {
                allHaveEpsilon = false;
                break;
            }
        }

        if (allHaveEpsilon) {
            result.insert("E");
        }
    }

    return result;
}
```

### FOLLOW集合计算

FOLLOW集合使用迭代法计算，直到集合不再变化为止：

```cpp
// 迭代计算FOLLOW集合
bool changed = true;
while (changed) {
    changed = false;
    for (const auto& prodPair : productions) {
        // 对每个产生式A -> αBβ
        // 1. 将FIRST(β) - {ε}加入FOLLOW(B)
        // 2. 若ε ∈ FIRST(β)或β为空，将FOLLOW(A)加入FOLLOW(B)
    }
}
```

### 计算结果示例

本文法的FIRST集合：

| 非终结符 | FIRST集合 |
|----------|-----------|
| program | { `{` } |
| stmt | { `if`, `while`, `ID`, `{` } |
| compoundstmt | { `{` } |
| stmts | { `if`, `while`, `ID`, `{`, `E` } |
| arithexpr | { `ID`, `NUM`, `(` } |
| arithexprprime | { `+`, `-`, `E` } |

---

## 3. 精确行号计算策略

### 问题背景

传统的行号计算方式会将空行也计入行号，导致错误提示的行号与用户看到的"有内容的行"不一致。

### 解决方案

本项目实现了**仅计算非空行**的行号策略：

```cpp
// Lexer类中的行号计算逻辑
class Lexer {
private:
    vector<string> lines;           // 按行分割的输入
    int contentLineNumber;          // 当前内容行号（仅计非空行）

    // 检查是否为空行
    bool isEmptyLine(const string& line) {
        for (char c : line) {
            if (c != ' ' && c != '\t' && c != '\r') {
                return false;
            }
        }
        return true;
    }

    vector<Token> tokenize() {
        while (currentLineIdx < lines.size()) {
            string& line = lines[currentLineIdx];

            // 跳过空行，不增加行号
            if (isEmptyLine(line)) {
                currentLineIdx++;
                continue;
            }

            // 非空行，增加内容行号
            contentLineNumber++;

            // 处理该行的tokens...
        }
    }
};
```

### 效果对比

假设输入为：
```
{

while ( ID == NUM )

{

ID = NUM

}

}
```

| 计算方式 | "ID = NUM"的行号 |
|----------|------------------|
| 传统方式（计空行） | 第7行 |
| 本项目方式（不计空行） | 第4行 |

本项目的方式与用户直觉一致，便于定位错误。

---

## 4. 同步恢复机制

### 错误恢复策略

本项目采用**Panic Mode Recovery（恐慌模式恢复）**策略，当遇到语法错误时：

1. **缺少终结符**：插入缺失的终结符，继续解析
2. **无对应产生式**：利用FOLLOW集合进行同步恢复

### 核心代码

```cpp
// 处理缺少终结符的错误
void handleMissingTerminal(const string& expected, TreeNode* parent) {
    int line = getPrevTokenLine();

    // 生成错误信息
    string errorMsg = "语法错误,第" + to_string(line) + "行,缺少\"" + expected + "\"";
    errors.push_back(errorMsg);

    // 插入缺失的终结符节点（错误恢复）
    parent->addChild(new TreeNode(expected));
}

// 处理分析表无对应项的错误
void handleError(const string& nonTerminal, const string& lookahead, TreeNode* node) {
    // 尝试使用FOLLOW集合进行同步恢复
    if (grammar.followSets[nonTerminal].count(lookahead)) {
        // 当前输入在FOLLOW集合中，使用空产生式
        node->addChild(new TreeNode("E"));
        return;
    }

    // 对于可以为空的非终结符，直接使用E
    // ...
}
```

### 错误恢复示例

输入（缺少分号）：
```
{ ID = NUM }
```

输出：
```
语法错误,第1行,缺少";"
program
    compoundstmt
        {
        stmts
            stmt
                assgstmt
                    ID
                    =
                    arithexpr
                        ...
                    ;        ← 自动插入
            stmts
                E
        }
```

---

## 5. 自动化测试脚本

### 功能说明

本项目提供了自动化测试脚本，支持批量测试和结果比对。

### 文件位置

```
test_automation/
├── run_tests.sh      # 测试脚本
├── main.cpp          # 测试主程序
├── LLparser.h        # 解析器头文件
├── testcases/        # 测试用例目录
├── expected/         # 预期输出目录
└── output/           # 实际输出目录
```

### 运行方法

```bash
# 进入测试目录
cd test_automation

# 添加测试用例
echo '{ ID = NUM ; }' > testcases/test1.txt
# 添加预期输出到 expected/test1.txt

# 运行测试
./run_tests.sh
```

### 脚本核心逻辑

```bash
# 运行单个测试
run_test() {
    local test_file=$1
    local test_name=$(basename "$test_file" .txt)
    local expected_file="$EXPECTED_DIR/${test_name}.txt"
    local output_file="$OUTPUT_DIR/${test_name}.txt"

    # 运行解析器
    cat "$test_file" | "$PARSER" > "$output_file" 2>&1

    # 比较输出
    if diff -q "$output_file" "$expected_file" > /dev/null 2>&1; then
        echo -e "${GREEN}[通过]${NC} $test_name"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}[失败]${NC} $test_name"
        FAILED=$((FAILED + 1))
    fi
}
```

### 输出示例

```
============================================
       LL(1)语法分析器自动化测试
============================================

[通过] test1
[通过] test2
[失败] test3
  差异：
  < 语法错误,第3行,缺少";"
  > 语法错误,第4行,缺少";"

============================================
测试结果统计
============================================
总计: 3
通过: 2
失败: 1
```

---

## 6. 递归遍历语法树输出

### 算法说明

语法树的输出采用**前序遍历（Pre-order Traversal）**算法，配合深度参数实现缩进：

```cpp
// 输出语法树（使用tab缩进）
void print(int depth = 0) const {
    // 输出缩进（depth个tab）
    for (int i = 0; i < depth; i++) {
        cout << "\t";
    }
    cout << symbol << endl;

    // 递归输出子节点
    for (const auto& child : children) {
        child->print(depth + 1);
    }
}
```

### 遍历顺序

```
        program (深度0)
           │
    compoundstmt (深度1)
      ┌────┼────┐
      {  stmts  } (深度2)
          │
        stmt (深度3)
          │
       assgstmt (深度4)
    ┌──┬──┬──┬──┐
   ID  = expr ; (深度5)
```

### 时间复杂度

- **时间复杂度**：O(n)，n为节点数
- **空间复杂度**：O(h)，h为树的高度（递归栈深度）

---

## 7. STL嵌套Map实现分析表

### 数据结构设计

LL(1)分析表使用**嵌套map**实现：

```cpp
// table[非终结符][终结符] = 产生式索引
map<string, map<string, int>> table;
```

### 查询操作

```cpp
// 获取产生式
vector<string> getProduction(const string& nonTerminal, const string& terminal) {
    if (table.count(nonTerminal) && table[nonTerminal].count(terminal)) {
        int idx = table[nonTerminal][terminal];
        return grammar.productions[nonTerminal][idx];
    }
    return vector<string>();  // 空表示错误
}

// 检查表项是否存在
bool hasEntry(const string& nonTerminal, const string& terminal) {
    return table.count(nonTerminal) && table[nonTerminal].count(terminal);
}
```

### 性能分析

| 操作 | 时间复杂度 |
|------|------------|
| 查询 | O(log n + log m) |
| 插入 | O(log n + log m) |

其中n为非终结符数量，m为终结符数量。

对于本文法：
- 非终结符：14个
- 终结符：23个
- 查询时间：约 O(log 14 + log 23) ≈ O(8)

### 分析表示例（部分）

| 非终结符 | `{` | `if` | `while` | `ID` |
|----------|-----|------|---------|------|
| program | 0 | - | - | - |
| stmt | 3 | 0 | 1 | 2 |
| stmts | 0 | 0 | 0 | 0 |
| compoundstmt | 0 | - | - | - |

---

## 8. 彩色终端可视化

### 功能说明

本项目提供了彩色终端可视化功能，不同深度的节点使用不同颜色显示。

### 文件位置

```
color_visualization/
└── main.cpp          # 彩色可视化版本
```

### 运行方法

```bash
# 进入目录
cd color_visualization

# 编译
g++ -std=c++11 -o color_parser main.cpp

# 运行
echo '{ ID = NUM ; }' | ./color_parser
```

### 颜色方案

```cpp
// ANSI颜色代码
namespace Color {
    const string RESET = "\033[0m";
    const string RED = "\033[31m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string BLUE = "\033[34m";
    const string MAGENTA = "\033[35m";
    const string CYAN = "\033[36m";
    // ...

    // 根据深度获取颜色
    string getColorByDepth(int depth) {
        static const string colors[] = {
            BRIGHT_RED,      // 深度0
            BRIGHT_GREEN,    // 深度1
            BRIGHT_YELLOW,   // 深度2
            BRIGHT_BLUE,     // 深度3
            // ...
        };
        return colors[depth % 10];
    }
}
```

### 输出效果

终端中会显示：
- **深度0（红色）**：program
- **深度1（绿色）**：compoundstmt
- **深度2（黄色）**：{, stmts, }
- **深度3（蓝色）**：stmt
- **深度4（品红）**：assgstmt
- 以此类推...

终结符使用**粗体**显示，非终结符使用正常字体。

---

## 9. Git版本控制

### 使用说明

本项目使用Git进行版本控制，便于追踪代码变更和协作开发。

### 常用Git命令

```bash
# 查看状态
git status

# 添加文件
git add LLparser.h

# 提交变更
git commit -m "LL(1) Parser"

# 查看历史
git log --oneline

# 创建分支
git branch feature-error-handling

# 切换分支
git checkout feature-error-handling
```

---

## 10. 模板方法设计模式

### 设计说明

本项目在解析流程中应用了**模板方法设计模式**，定义了解析的骨架算法，将某些步骤延迟到子类或具体实现中。

### 解析流程模板

```cpp
TreeNode* parse(const string& prog) {
    // 步骤1：词法分析
    Lexer lexer(prog);
    tokens = lexer.tokenize();

    // 步骤2：初始化
    currentPos = 0;
    errors.clear();

    // 步骤3：语法分析（核心算法）
    TreeNode* root = parseNonTerminal("program");

    // 步骤4：输出错误信息
    for (const auto& err : errors) {
        cout << err << endl;
    }

    return root;
}
```

### parseNonTerminal的模板结构

```cpp
TreeNode* parseNonTerminal(const string& nonTerminal) {
    TreeNode* node = new TreeNode(nonTerminal);
    string lookahead = currentToken().type;

    // 步骤1：查找分析表
    if (!parseTable.hasEntry(nonTerminal, lookahead)) {
        handleError(nonTerminal, lookahead, node);  // 可扩展的错误处理
        return node;
    }

    // 步骤2：获取产生式
    vector<string> production = parseTable.getProduction(nonTerminal, lookahead);

    // 步骤3：处理空产生式
    if (production.size() == 1 && production[0] == "E") {
        node->addChild(new TreeNode("E"));
        return node;
    }

    // 步骤4：处理产生式中的每个符号
    for (const auto& symbol : production) {
        if (grammar.nonTerminals.count(symbol)) {
            // 非终结符：递归解析
            TreeNode* child = parseNonTerminal(symbol);
            node->addChild(child);
        } else {
            // 终结符：匹配或错误处理
            if (currentToken().type == symbol) {
                node->addChild(new TreeNode(symbol));
                advance();
            } else {
                handleMissingTerminal(symbol, node);  // 可扩展的错误处理
            }
        }
    }

    return node;
}
```

### 扩展性示例

如需添加语义分析功能，只需：

```cpp
// 扩展TreeNode类
class SemanticTreeNode : public TreeNode {
public:
    string semanticValue;  // 语义值
    string type;           // 类型信息
};

// 扩展handleMissingTerminal方法
void handleMissingTerminal(const string& expected, TreeNode* parent) {
    // 基础错误处理...

    // 添加语义错误记录
    recordSemanticError(expected, parent);
}
```

---

## 附录：项目结构

```
lab_2/
├── LLparser.h              # 主解析器代码
├── main.cpp                # 测试主程序
├── innovations.md          # 实践亮点
├── supp.md                 # 补充文档（本文件）
├── doc.md                  # 任务说明
├── static/
│   └── example.jpg         # 示例图片
├── test_automation/        # 自动化测试
│   ├── run_tests.sh
│   ├── main.cpp
│   ├── LLparser.h
│   ├── testcases/
│   ├── expected/
│   └── output/
└── color_visualization/    # 彩色可视化
    └── main.cpp
```

---

## 编译与运行

### 主程序

```bash
# 编译
g++ -std=c++11 -o parser main.cpp

# 运行
echo '{ ID = NUM ; }' | ./parser
```

### 自动化测试

```bash
cd test_automation
./run_tests.sh
```

### 彩色可视化

```bash
cd color_visualization
g++ -std=c++11 -o color_parser main.cpp
echo '{ ID = NUM ; }' | ./color_parser
```
