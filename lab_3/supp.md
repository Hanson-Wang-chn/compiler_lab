# LR(1)语法分析器补充文档

本文档对应`innovations.md`中的10个实践亮点，提供详细的技术分析、代码说明和运行方法。

---

## 目录

1. [LR(1)分析算法](#1-lr1分析算法)
2. [面向对象设计](#2-面向对象设计)
3. [自动化表生成](#3-自动化表生成)
4. [错误恢复机制](#4-错误恢复机制)
5. [FIRST/FOLLOW集计算](#5-firstfollow集计算)
6. [最右推导输出](#6-最右推导输出)
7. [集成式词法分析器](#7-集成式词法分析器)
8. [自动化测试脚本](#8-自动化测试脚本)
9. [高效数据结构](#9-高效数据结构)
10. [模块化分层设计](#10-模块化分层设计)

---

## 1. LR(1)分析算法

### 1.1 技术概述

LR(1)分析是一种强大的自底向上语法分析技术，其中"1"表示向前看一个符号。与SLR(1)相比，LR(1)分析器为每个项目维护一个精确的向前看符号，能够处理更大范围的文法，减少移入-规约和规约-规约冲突。

### 1.2 核心算法

#### LR(1)项目定义

LR(1)项目的形式为 `[A -> α·β, a]`，其中：
- `A -> αβ` 是一个产生式
- `·` 表示当前分析位置
- `a` 是向前看符号，表示在规约时期望看到的下一个输入符号

```cpp
// LR(1)项目类定义
class LR1Item {
public:
    Production prod;    // 对应的产生式
    int dotPos;         // 点的位置（0表示在最左边）
    Symbol lookahead;   // 向前看符号

    // 获取点后面的符号
    Symbol getSymbolAfterDot() const {
        if (dotPos < (int)prod.right.size() &&
            !(prod.right.size() == 1 && prod.right[0].name == "E")) {
            return prod.right[dotPos];
        }
        return Symbol("", true);
    }

    // 判断点是否在产生式末尾（规约项目）
    bool isComplete() const {
        if (prod.right.size() == 1 && prod.right[0].name == "E") {
            return true;  // 空产生式
        }
        return dotPos >= (int)prod.right.size();
    }
};
```

#### 闭包计算

闭包操作是LR(1)分析的核心，用于计算一个项目集的完整状态：

```cpp
LR1State closure(const set<LR1Item>& items) {
    LR1State state;
    state.items = items;

    queue<LR1Item> workList;
    for (const LR1Item& item : items) {
        workList.push(item);
    }

    while (!workList.empty()) {
        LR1Item item = workList.front();
        workList.pop();

        Symbol B = item.getSymbolAfterDot();
        if (B.name.empty() || B.isTerminal) continue;

        // 构造βa串，用于计算新的向前看符号
        vector<Symbol> betaA;
        for (int i = item.dotPos + 1; i < (int)item.prod.right.size(); i++) {
            if (item.prod.right[i].name != "E") {
                betaA.push_back(item.prod.right[i]);
            }
        }
        betaA.push_back(item.lookahead);

        // 计算FIRST(βa)作为新的向前看符号
        set<Symbol> firstBetaA = getFirstOfString(betaA, 0);

        // 对每个B的产生式，添加新项目
        for (const Production& prod : productions) {
            if (prod.left == B) {
                for (const Symbol& la : firstBetaA) {
                    if (la.name == "E") continue;
                    LR1Item newItem(prod, 0, la);
                    if (state.items.find(newItem) == state.items.end()) {
                        state.items.insert(newItem);
                        workList.push(newItem);
                    }
                }
            }
        }
    }
    return state;
}
```

### 1.3 分析表构建

ACTION表和GOTO表的构建规则：

1. **移入动作**：如果项目 `[A -> α·aβ, b]` 在状态I中，且 `GOTO(I, a) = J`，则 `ACTION[I, a] = shift J`

2. **规约动作**：如果项目 `[A -> α·, a]` 在状态I中，且A不是增广起始符号，则 `ACTION[I, a] = reduce A -> α`

3. **接受动作**：如果项目 `[S' -> S·, $]` 在状态I中，则 `ACTION[I, $] = accept`

4. **GOTO表**：对于非终结符A，如果 `GOTO(I, A) = J`，则 `GOTO[I, A] = J`

### 1.4 运行方法

LR(1)分析器已集成在`LRparser.h`中，编译运行主程序即可：

```bash
g++ -std=c++11 -o test_parser test_main.cpp
echo '{ ID = NUM ; }' | ./test_parser
```

---

## 2. 面向对象设计

### 2.1 设计理念

本项目采用面向对象（OOP）设计方法，将语法分析器的各个组件抽象为独立的类，实现了高内聚、低耦合的代码结构。

### 2.2 类层次结构

```
LRParser (主分析器类)
    │
    ├── Symbol (符号类)
    │   └── 封装终结符和非终结符
    │
    ├── Production (产生式类)
    │   └── 封装文法产生式
    │
    ├── LR1Item (LR(1)项目类)
    │   └── 封装带向前看符号的分析项目
    │
    ├── LR1State (状态类)
    │   └── 封装LR(1)自动机状态
    │
    └── Token (词法单元类)
        └── 封装输入的词法单元
```

### 2.3 类设计详解

#### Symbol类

```cpp
class Symbol {
public:
    string name;        // 符号名称
    bool isTerminal;    // 是否为终结符

    // 构造函数
    Symbol() : name(""), isTerminal(true) {}
    Symbol(const string& n, bool term) : name(n), isTerminal(term) {}

    // 比较运算符，用于在set和map中存储
    bool operator==(const Symbol& other) const;
    bool operator<(const Symbol& other) const;
    bool operator!=(const Symbol& other) const;
};
```

#### Production类

```cpp
class Production {
public:
    Symbol left;                // 产生式左部（非终结符）
    vector<Symbol> right;       // 产生式右部（符号序列）
    int id;                     // 产生式编号

    Production() : id(-1) {}
    Production(const Symbol& l, const vector<Symbol>& r, int i);
};
```

### 2.4 OOP优势

1. **封装性**：每个类封装了自己的数据和操作，对外提供清晰的接口
2. **可维护性**：修改某个类的内部实现不会影响其他类
3. **可扩展性**：可以通过继承或组合方式扩展功能
4. **可测试性**：每个类可以独立进行单元测试

---

## 3. 自动化表生成

### 3.1 技术概述

本项目实现了LR(1)分析表的自动生成，避免了手工构造分析表的繁琐和易错性。

### 3.2 表生成流程

```
文法定义 → FIRST/FOLLOW集计算 → LR(1)项目集族构建 → ACTION/GOTO表生成
```

### 3.3 自动机构建算法

```cpp
void buildLR1Automaton() {
    // 初始状态：[S' -> ·S, $]
    LR1Item startItem(productions[0], 0, Symbol("$", true));
    set<LR1Item> startItems;
    startItems.insert(startItem);
    LR1State startState = closure(startItems);
    startState.id = 0;
    states.push_back(startState);

    // 收集所有文法符号
    set<Symbol> allSymbols;
    for (const Symbol& t : terminals) {
        if (t.name != "$") allSymbols.insert(t);
    }
    for (const Symbol& nt : nonTerminals) {
        allSymbols.insert(nt);
    }

    // BFS构建状态集
    queue<int> workList;
    workList.push(0);
    map<set<LR1Item>, int> stateMap;
    stateMap[startState.items] = 0;

    while (!workList.empty()) {
        int stateId = workList.front();
        workList.pop();

        for (const Symbol& X : allSymbols) {
            LR1State newState = gotoFunc(states[stateId], X);
            if (newState.items.empty()) continue;

            auto it = stateMap.find(newState.items);
            if (it == stateMap.end()) {
                // 发现新状态
                newState.id = states.size();
                stateMap[newState.items] = newState.id;
                states.push_back(newState);
                workList.push(newState.id);
                gotoTable[make_pair(stateId, X)] = newState.id;
            } else {
                gotoTable[make_pair(stateId, X)] = it->second;
            }
        }
    }
}
```

### 3.4 可视化工具

提供了ACTION/GOTO表可视化工具，位于`table_visualizer/`目录：

```bash
cd table_visualizer
make
./table_visualizer
```

输出示例：
```
ACTION表统计
==================
状态数量: 87
ACTION表项数量: 156
移入动作数量: 98
规约动作数量: 57
接受动作数量: 1
```

---

## 4. 错误恢复机制

### 4.1 技术概述

本项目实现了基于符号插入的恐慌模式错误恢复机制，能够在检测到语法错误时自动进行修正并继续分析。

### 4.2 错误类型识别

分析器能够识别多种常见的语法错误：

| 错误类型 | 检测条件 | 恢复策略 |
|---------|---------|---------|
| 缺少分号 | 期望`;`但未得到 | 插入`;` |
| 缺少右括号 | 期望`)`但未得到 | 插入`)` |
| 缺少右花括号 | 期望`}`但未得到 | 插入`}` |
| 缺少then | 期望`then`但未得到 | 插入`then` |
| 缺少else | 期望`else`但未得到 | 插入`else` |
| 意外符号 | 其他情况 | 跳过当前符号 |

### 4.3 错误处理代码

```cpp
void handleError(int state, const Token& tok, stack<int>& stateStack,
                 stack<Symbol>& symbolStack) {
    // 分析当前状态期望的符号
    set<string> expected;
    for (auto& entry : actionTable) {
        if (entry.first.first == state && entry.second != 0) {
            expected.insert(entry.first.second.name);
        }
    }

    // 判断错误类型并恢复
    if (expected.find(";") != expected.end() && tok.value != ";") {
        // 缺少分号 - 使用上一个token的行号
        string errorMsg = "语法错误，第" + to_string(lastTokenLine) + "行，缺少\";\"";
        errors.push_back(errorMsg);

        // 插入分号继续分析
        Token semicolon(";", lastTokenLine);
        tokens.insert(tokens.begin() + currentToken, semicolon);
    }
    // ... 其他错误类型处理
}
```

### 4.4 行号追踪

为了准确报告错误位置，分析器维护了上一个词法单元的行号：

```cpp
int lastTokenLine;  // 上一个词法单元的行号

void advance() {
    if (currentToken < (int)tokens.size()) {
        lastTokenLine = tokens[currentToken].line;
        currentToken++;
    }
}
```

### 4.5 错误消息格式

错误消息采用统一格式：`语法错误，第X行，错误描述`

示例输出：
```
语法错误，第4行，缺少";"
```

---

## 5. FIRST/FOLLOW集计算

### 5.1 技术概述

FIRST集和FOLLOW集是构造LR分析表的基础，本项目实现了迭代算法自动计算这两个集合。

### 5.2 FIRST集计算规则

1. 如果X是终结符，则 FIRST(X) = {X}
2. 如果X是非终结符，且 X -> Y1Y2...Yk 是产生式：
   - 将FIRST(Y1)中的非ε符号加入FIRST(X)
   - 如果ε∈FIRST(Y1)，将FIRST(Y2)中的非ε符号加入FIRST(X)
   - 以此类推
3. 如果X -> ε，则ε∈FIRST(X)

```cpp
void computeFirstSets() {
    // 终结符的FIRST集是其自身
    for (const Symbol& t : terminals) {
        firstSet[t].insert(t);
    }
    Symbol epsilon("E", true);
    firstSet[epsilon].insert(epsilon);

    // 迭代计算非终结符的FIRST集
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Production& prod : productions) {
            set<Symbol>& first = firstSet[prod.left];
            size_t oldSize = first.size();

            bool allHaveEpsilon = true;
            for (const Symbol& sym : prod.right) {
                if (sym.name == "E") {
                    first.insert(Symbol("E", true));
                    break;
                }

                for (const Symbol& f : firstSet[sym]) {
                    if (f.name != "E") {
                        first.insert(f);
                    }
                }

                if (firstSet[sym].find(Symbol("E", true)) == firstSet[sym].end()) {
                    allHaveEpsilon = false;
                    break;
                }
            }

            if (first.size() != oldSize) {
                changed = true;
            }
        }
    }
}
```

### 5.3 FOLLOW集计算规则

1. 将$加入FOLLOW(S)，其中S是起始符号
2. 如果存在产生式 A -> αBβ：
   - 将FIRST(β) - {ε}加入FOLLOW(B)
   - 如果ε∈FIRST(β)，将FOLLOW(A)加入FOLLOW(B)
3. 如果存在产生式 A -> αB，将FOLLOW(A)加入FOLLOW(B)

### 5.4 可视化工具

提供了FIRST/FOLLOW集输出工具，位于`first_follow_tool/`目录：

```bash
cd first_follow_tool
make
./first_follow
```

输出示例：
```
FIRST 集
========================================
FIRST(program) = { { }
FIRST(stmt) = { ID, if, while, { }
FIRST(compoundstmt) = { { }
FIRST(stmts) = { E, ID, if, while, { }
...

FOLLOW 集
========================================
FOLLOW(program) = { $ }
FOLLOW(stmt) = { ID, else, if, while, {, } }
FOLLOW(compoundstmt) = { $, ID, else, if, while, {, } }
...
```

---

## 6. 最右推导输出

### 6.1 技术概述

本项目实现了从LR分析结果反向生成最右推导序列的算法，这是因为LR分析是自底向上的，而最右推导是自顶向下的。

### 6.2 推导生成原理

LR分析过程中，每次规约操作对应最右推导的一个逆向步骤。因此：

1. 记录所有规约操作使用的产生式编号
2. 将规约序列逆序
3. 从起始符号开始，按逆序应用产生式展开

### 6.3 实现代码

```cpp
void generateDerivation(const vector<int>& reductions) {
    // 从起始符号开始
    vector<Symbol> form;
    form.push_back(startSymbol);
    derivation.push_back(sententialFormToString(form));

    // 逆序处理规约序列
    for (int i = reductions.size() - 1; i >= 0; i--) {
        int prodId = reductions[i];
        if (prodId == 0) continue;  // 跳过增广产生式

        const Production& prod = productions[prodId];

        // 在当前句型中找到最右边可以展开的非终结符
        int pos = -1;
        for (int j = form.size() - 1; j >= 0; j--) {
            if (form[j] == prod.left) {
                pos = j;
                break;
            }
        }

        if (pos != -1) {
            // 用产生式右部替换左部
            vector<Symbol> newForm;
            for (int j = 0; j < pos; j++) {
                newForm.push_back(form[j]);
            }

            // 插入产生式右部（除非是空产生式）
            if (!(prod.right.size() == 1 && prod.right[0].name == "E")) {
                for (const Symbol& s : prod.right) {
                    newForm.push_back(s);
                }
            }

            for (int j = pos + 1; j < (int)form.size(); j++) {
                newForm.push_back(form[j]);
            }

            form = newForm;
            derivation.push_back(sententialFormToString(form));
        }
    }
}
```

### 6.4 输出格式

推导过程按以下格式输出：
- 每行显示一个句型
- 除最后一行外，每行末尾添加` => `和换行
- 最后一行为最终的终结符串

示例：
```
program =>
compoundstmt =>
{ stmts } =>
{ stmt stmts } =>
{ stmt } =>
{ assgstmt } =>
{ ID = arithexpr ; } =>
...
{ ID = NUM ; }
```

---

## 7. 集成式词法分析器

### 7.1 技术概述

本项目集成了一个轻量级词法分析器，将输入源程序转换为词法单元序列，为语法分析提供输入。

### 7.2 词法单元类型

| 类型 | 示例 | 说明 |
|-----|------|-----|
| 关键字 | if, then, else, while | 保留字 |
| 标识符 | ID | 变量名 |
| 数字 | NUM | 整数常量 |
| 运算符 | +, -, *, /, <, >, <=, >=, == | 算术和比较运算符 |
| 分隔符 | {, }, (, ), ;, = | 标点符号 |

### 7.3 词法分析实现

```cpp
void tokenize(const string& prog) {
    tokens.clear();
    int line = 1;
    int i = 0;
    int n = prog.size();

    while (i < n) {
        // 跳过空白符，但记录换行
        while (i < n && (prog[i] == ' ' || prog[i] == '\t' ||
                        prog[i] == '\n' || prog[i] == '\r')) {
            if (prog[i] == '\n') {
                line++;
            }
            i++;
        }
        if (i >= n) break;

        string token;

        // 双字符运算符
        if (i + 1 < n) {
            string two = prog.substr(i, 2);
            if (two == "<=" || two == ">=" || two == "==") {
                tokens.push_back(Token(two, line));
                i += 2;
                continue;
            }
        }

        // 单字符符号
        char c = prog[i];
        if (c == '{' || c == '}' || c == '(' || c == ')' ||
            c == '=' || c == ';' || c == '<' || c == '>' ||
            c == '+' || c == '-' || c == '*' || c == '/') {
            tokens.push_back(Token(string(1, c), line));
            i++;
            continue;
        }

        // 标识符或关键字
        if (isalpha(c) || c == '_') {
            while (i < n && (isalnum(prog[i]) || prog[i] == '_')) {
                token += prog[i];
                i++;
            }
            if (token == "if" || token == "then" || token == "else" ||
                token == "while") {
                tokens.push_back(Token(token, line));
            } else if (token == "ID" || token == "NUM") {
                tokens.push_back(Token(token, line));
            } else {
                tokens.push_back(Token("ID", line));
            }
            continue;
        }

        // 数字
        if (isdigit(c)) {
            while (i < n && isdigit(prog[i])) {
                token += prog[i];
                i++;
            }
            tokens.push_back(Token("NUM", line));
            continue;
        }

        i++;
    }

    // 添加结束符
    tokens.push_back(Token("$", line));
}
```

### 7.4 行号追踪

词法分析器在处理过程中维护行号信息，用于后续的错误报告。每个词法单元都记录其所在行号。

---

## 8. 自动化测试脚本

### 8.1 技术概述

本项目提供了自动化测试脚本，支持批量测试、结果对比和测试报告生成。

### 8.2 目录结构

```
test_script/
├── run_tests.sh        # 主测试脚本
├── testcases/          # 测试输入文件目录
│   └── *.txt
├── expected/           # 预期输出文件目录
│   └── *.txt
└── output/            # 实际输出文件目录
    └── *.txt
```

### 8.3 使用方法

```bash
cd test_script

# 生成测试用例模板
./run_tests.sh -g

# 运行所有测试
./run_tests.sh

# 显示详细差异
./run_tests.sh -v

# 清理输出目录
./run_tests.sh -c

# 显示帮助
./run_tests.sh -h
```

### 8.4 测试报告

测试完成后会生成统计报告：

```
==========================================
     LR(1)语法分析器自动化测试
==========================================

[通过] test_simple
[通过] test_addition
[失败] test_error

==========================================
              测试统计
==========================================
总计: 3
通过: 2
失败: 1
```

### 8.5 添加新测试用例

1. 在`testcases/`目录创建输入文件，如`test_new.txt`
2. 在`expected/`目录创建对应的预期输出文件`test_new.txt`
3. 运行测试脚本验证

---

## 9. 高效数据结构

### 9.1 技术概述

本项目采用STL容器作为核心数据结构，利用其高效的实现来提升分析器性能。

### 9.2 主要数据结构

| 数据结构 | STL容器 | 用途 | 时间复杂度 |
|---------|--------|------|-----------|
| 符号集合 | set<Symbol> | 存储终结符/非终结符 | O(log n)查找 |
| FIRST/FOLLOW集 | map<Symbol, set<Symbol>> | 存储集合映射 | O(log n)访问 |
| 状态集合 | vector<LR1State> | 存储LR(1)状态 | O(1)索引访问 |
| 分析表 | map<pair<int,Symbol>, int> | ACTION/GOTO表 | O(log n)访问 |
| 分析栈 | stack<int>, stack<Symbol> | 状态栈和符号栈 | O(1)操作 |

### 9.3 性能优化

#### 状态去重

使用map以项目集作为键来检测重复状态：

```cpp
map<set<LR1Item>, int> stateMap;

// 检查新状态是否已存在
auto it = stateMap.find(newState.items);
if (it == stateMap.end()) {
    // 新状态
    newState.id = states.size();
    stateMap[newState.items] = newState.id;
    states.push_back(newState);
} else {
    // 已存在的状态
    gotoTable[make_pair(stateId, X)] = it->second;
}
```

#### BFS状态构建

使用队列进行广度优先搜索构建状态：

```cpp
queue<int> workList;
workList.push(0);

while (!workList.empty()) {
    int stateId = workList.front();
    workList.pop();
    // 处理状态转移...
}
```

### 9.4 空间效率

- 产生式右部使用vector存储，支持变长
- 项目集使用set自动去重
- 使用引用传递避免不必要的复制

---

## 10. 模块化分层设计

### 10.1 技术概述

本项目采用模块化分层设计，将分析器划分为五个主要模块，每个模块负责特定功能。

### 10.2 模块划分

```
┌─────────────────────────────────────────────────┐
│                   主控制模块                      │
│               Analysis()函数                      │
└─────────────────────────────────────────────────┘
                        │
        ┌───────────────┼───────────────┐
        ▼               ▼               ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│   词法分析    │ │   语法分析    │ │   错误处理    │
│  tokenize()  │ │    parse()   │ │ handleError()│
└──────────────┘ └──────────────┘ └──────────────┘
        │               │               │
        └───────────────┼───────────────┘
                        ▼
        ┌───────────────────────────────┐
        │        输出生成模块            │
        │ generateDerivation()          │
        │ printResult()                 │
        └───────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────┐
│                   基础设施                       │
│    Symbol, Production, LR1Item, LR1State        │
└─────────────────────────────────────────────────┘
```

### 10.3 模块职责

| 模块 | 职责 | 主要函数/类 |
|-----|------|------------|
| 符号管理 | 定义和管理文法符号 | Symbol, Production |
| 文法定义 | 初始化产生式和符号集 | initGrammar() |
| 自动机构建 | 构建LR(1)状态和分析表 | buildLR1Automaton(), buildParsingTable() |
| 分析驱动 | 执行LR分析算法 | parse(), tokenize() |
| 错误处理 | 检测和恢复语法错误 | handleError() |

### 10.4 模块间接口

各模块通过类的成员变量进行通信：

```cpp
class LRParser {
private:
    // 文法模块数据
    vector<Production> productions;
    set<Symbol> terminals;
    set<Symbol> nonTerminals;

    // 自动机模块数据
    vector<LR1State> states;
    map<pair<int, Symbol>, int> gotoTable;
    map<pair<int, Symbol>, int> actionTable;

    // 分析模块数据
    vector<Token> tokens;
    int currentToken;

    // 错误处理模块数据
    vector<string> errors;

    // 输出模块数据
    vector<string> derivation;
};
```

### 10.5 可扩展性

模块化设计使得系统易于扩展：

1. **添加新的错误类型**：只需修改handleError()函数
2. **支持新的输出格式**：添加新的输出函数
3. **修改文法**：只需修改initGrammar()函数
4. **优化算法**：可以独立优化各模块而不影响其他部分

---

## 附录：编译与运行

### 主程序编译

```bash
cd /home/whs/compiler_lab/lab_3
g++ -std=c++11 -o test_parser test_main.cpp
```

### 工具编译

```bash
# FIRST/FOLLOW集工具
cd first_follow_tool
make

# ACTION/GOTO表可视化工具
cd table_visualizer
make
```

### 运行测试

```bash
# 运行主程序
echo '{ ID = NUM ; }' | ./test_parser

# 运行自动化测试
cd test_script
chmod +x run_tests.sh
./run_tests.sh
```
