# 补充文档

本文档详细说明了Translation Schema项目的10个实践亮点，包括技术分析、设计原理和相关代码说明。

---

## 1. 面向对象编程技术

### 技术说明

本项目采用面向对象编程（OOP）方法，将翻译模式的各个组件封装为独立的类，实现了高内聚、低耦合的软件架构。

### 类结构设计

项目包含以下核心类：

1. **Lexer类**：词法分析器，负责将输入字符流转换为词法单元（Token）序列
2. **Parser类**：语法分析器和解释器，实现递归下降解析和程序执行
3. **Token结构体**：词法单元的数据表示
4. **Value结构体**：运行时值的统一表示
5. **Symbol结构体**：符号表项的数据结构

### 核心代码分析

```cpp
// 词法分析器类 - 封装了所有词法分析逻辑
class Lexer {
private:
    string input;           // 输入字符串
    size_t pos;             // 当前位置
    int currentLine;        // 当前行号

    void skipWhitespace();              // 私有方法：跳过空白
    Token readIdentifierOrKeyword();    // 私有方法：读取标识符
    Token readNumber();                 // 私有方法：读取数字

public:
    Lexer(const string& src);           // 构造函数
    Token nextToken();                  // 公共接口：获取下一个Token
    int getLine() const;                // 公共接口：获取行号
};
```

### 设计优势

- **封装性**：内部实现细节对外隐藏，只暴露必要的公共接口
- **可维护性**：修改某个类的实现不会影响其他类
- **可复用性**：各类可以独立测试和复用

---

## 2. 递归下降解析方法

### 技术说明

递归下降解析是一种自顶向下的语法分析技术，每个非终结符对应一个解析函数，函数之间通过递归调用实现语法规则的匹配。

### 文法到代码的映射

以算术表达式文法为例：

```
arithexpr -> multexpr arithexprprime
arithexprprime -> + multexpr arithexprprime | - multexpr arithexprprime | E
```

对应的解析函数：

```cpp
// 消除左递归后的实现
Value parseArithExpr(int line) {
    Value result = parseMultExpr(line);  // 解析第一个multexpr

    // 处理arithexprprime：循环处理 +/- 运算
    while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        TokenType op = currentToken.type;
        advance();
        Value right = parseMultExpr(line);

        // 执行运算
        bool isReal = result.isReal || right.isReal;
        double val = (op == TOKEN_PLUS) ?
            result.toDouble() + right.toDouble() :
            result.toDouble() - right.toDouble();

        result = isReal ? Value(val) : Value((int)val);
    }
    return result;
}
```

### 完整的文法规则映射表

| 文法规则 | 对应函数 |
|---------|---------|
| program -> decls compoundstmt | parseProgram() |
| decls -> decl ; decls \| E | parseDecls() |
| decl -> int ID = INTNUM \| real ID = REALNUM | parseDecl() |
| compoundstmt -> { stmts } | parseCompoundStmt() |
| stmts -> stmt stmts \| E | parseStmts() |
| stmt -> ifstmt \| assgstmt \| compoundstmt | parseStmt() |
| ifstmt -> if ( boolexpr ) then stmt else stmt | parseIfStmt() |
| assgstmt -> ID = arithexpr ; | parseAssgStmt() |
| boolexpr -> arithexpr boolop arithexpr | parseBoolExpr() |
| arithexpr -> multexpr arithexprprime | parseArithExpr() |
| multexpr -> simpleexpr multexprprime | parseMultExpr() |
| simpleexpr -> ID \| INTNUM \| REALNUM \| ( arithexpr ) | parseSimpleExpr() |

---

## 3. 符号表哈希表优化

### 技术说明

符号表使用C++ STL的`map<string, Symbol>`实现，底层采用红黑树结构，提供O(log n)的查询时间复杂度。对于小规模程序，实际表现接近O(1)。

### 数据结构设计

```cpp
// 符号表项结构体
struct Symbol {
    string name;        // 变量名
    bool isReal;        // 是否为实数类型
    Value value;        // 变量值
    int declareLine;    // 声明所在行号
};

// 符号表定义
map<string, Symbol> symbolTable;    // 主符号表
vector<string> varOrder;            // 变量声明顺序（用于有序输出）
```

### 符号表操作

```cpp
// 变量声明时插入符号表
symbolTable[varName] = Symbol(varName, true, Value(val), line);
varOrder.push_back(varName);

// 变量使用时查询符号表
if (symbolTable.find(varName) == symbolTable.end()) {
    addError(line, "undefined variable: " + varName);
}

// 变量赋值时更新符号表
Symbol& sym = symbolTable[varName];
sym.value = newValue;
```

### 设计优势

- **快速查找**：变量查询时间复杂度为O(log n)
- **有序输出**：使用额外的vector维护声明顺序，确保输出顺序与声明顺序一致
- **类型安全**：每个符号项都存储了类型信息，便于类型检查

---

## 4. 双类型值存储机制

### 技术说明

设计了统一的Value结构体来存储整数和实数值，通过标志位区分类型，实现了类型安全的运算和转换。

### 数据结构设计

```cpp
struct Value {
    bool isReal;        // 标记是否为实数类型
    int intVal;         // 整数值
    double realVal;     // 实数值

    // 默认构造函数
    Value() : isReal(false), intVal(0), realVal(0.0) {}

    // 整数构造函数
    Value(int v) : isReal(false), intVal(v), realVal(0.0) {}

    // 实数构造函数
    Value(double v) : isReal(true), intVal(0), realVal(v) {}

    // 统一转换为double用于计算
    double toDouble() const {
        return isReal ? realVal : (double)intVal;
    }
};
```

### 类型转换规则

1. **整数到实数**：允许隐式转换（int -> real）
2. **实数到整数**：不允许隐式转换，会产生语义错误
3. **混合运算**：结果类型为实数

### 类型检查示例

```cpp
// 声明时的类型检查
if (!isReal && currentToken.type == TOKEN_REALNUM) {
    // 实数不能赋给整数类型变量
    addError(line, "realnum can not be translated into int type");
}

// 运算时的类型推导
bool isReal = result.isReal || right.isReal;  // 任一操作数为实数，结果为实数
```

---

## 5. 行号追踪与错误收集机制

### 技术说明

实现了完整的错误处理机制，包括行号追踪、错误信息收集和格式化输出。

### 行号追踪实现

```cpp
// 在词法分析器中追踪行号
void skipWhitespace() {
    while (pos < input.length()) {
        if (input[pos] == '\n') {
            currentLine++;  // 遇到换行符时增加行号
            pos++;
        } else if (isspace(input[pos])) {
            pos++;
        } else {
            break;
        }
    }
}

// 每个Token都记录其所在行号
Token(TokenType t, const string& v, int l) : type(t), value(v), line(l) {}
```

### 错误收集机制

```cpp
// 错误信息列表
vector<string> errors;
bool hasError;

// 添加错误信息
void addError(int line, const string& msg) {
    stringstream ss;
    ss << "error message:line " << line << "," << msg;
    errors.push_back(ss.str());
    hasError = true;
}
```

### 错误类型

本项目检测以下语义错误：

| 错误类型 | 错误信息 |
|---------|---------|
| 类型不匹配 | realnum can not be translated into int type |
| 除以零 | division by zero |
| 未定义变量 | undefined variable: xxx |

---

## 6. Shell自动化测试脚本

### 技术说明

编写了Shell脚本实现自动化测试，支持批量执行测试用例并验证输出结果。

### 运行方法

```bash
cd test_scripts
chmod +x run_tests.sh
./run_tests.sh
```

### 脚本功能

1. 自动编译TranslationSchema.h
2. 遍历测试用例目录
3. 执行每个测试用例
4. 比较实际输出与期望输出
5. 生成测试报告

详细代码见`test_scripts/`目录。

---

## 7. 运算符优先级分层解析算法

### 技术说明

通过文法分层设计实现运算符优先级，加减法在`arithexpr`层处理，乘除法在`multexpr`层处理，保证了正确的运算顺序。

### 优先级层次

```
低优先级  arithexpr  ->  + -  运算
    |
    v
高优先级  multexpr   ->  * /  运算
    |
    v
最高优先级 simpleexpr ->  括号、常量、变量
```

### 解析过程示例

对于表达式 `a + b * c`：

1. `parseArithExpr()` 调用 `parseMultExpr()` 解析 `a`
2. 遇到 `+`，继续调用 `parseMultExpr()` 解析 `b * c`
3. 在 `parseMultExpr()` 中，先解析 `b`，遇到 `*`，解析 `c`
4. 先计算 `b * c`，返回结果
5. 再计算 `a + (b * c)`

### 代码实现

```cpp
// 乘除法优先级高于加减法
Value parseMultExpr(int line) {
    Value result = parseSimpleExpr(line);  // 先解析操作数

    while (currentToken.type == TOKEN_MUL || currentToken.type == TOKEN_DIV) {
        TokenType op = currentToken.type;
        advance();
        Value right = parseSimpleExpr(line);

        // 立即执行乘除运算
        if (op == TOKEN_MUL) {
            result = Value(result.toDouble() * right.toDouble());
        } else {
            if (right.toDouble() == 0) {
                addError(line, "division by zero");
            }
            result = Value(result.toDouble() / right.toDouble());
        }
    }
    return result;
}
```

---

## 8. 分支跳过解析技术

### 技术说明

在if-else语句执行时，只执行满足条件的分支，但仍需解析跳过的分支以保持语法正确性。设计了专门的skip系列函数实现这一功能。

### 设计原理

```cpp
void parseIfStmt() {
    match(TOKEN_IF);
    expect(TOKEN_LPAREN);
    bool condition = parseBoolExpr();  // 求值条件表达式
    expect(TOKEN_RPAREN);
    expect(TOKEN_THEN);

    if (condition) {
        parseStmt();       // 执行then分支
        expect(TOKEN_ELSE);
        skipStmt();        // 跳过else分支（不执行，但要解析）
    } else {
        skipStmt();        // 跳过then分支
        expect(TOKEN_ELSE);
        parseStmt();       // 执行else分支
    }
}
```

### Skip函数族

```cpp
void skipStmt();           // 跳过任意语句
void skipIfStmt();         // 跳过if语句
void skipAssgStmt();       // 跳过赋值语句
void skipCompoundStmt();   // 跳过复合语句
void skipBoolExpr();       // 跳过布尔表达式
void skipArithExpr();      // 跳过算术表达式
void skipMultExpr();       // 跳过乘法表达式
void skipSimpleExpr();     // 跳过简单表达式
```

### 设计优势

- **避免副作用**：跳过的分支中的赋值语句不会执行
- **语法检查**：仍然验证跳过分支的语法正确性
- **保持解析状态**：正确移动Token指针到分支结束位置

---

## 9. 统一的中文注释规范

### 技术说明

项目采用统一的中文注释规范，提高代码可读性和可维护性。

### 注释规范

1. **文件头注释**：说明文件用途
```cpp
// C语言翻译模式实现
// Translation Schema for C Programming Language
```

2. **模块分隔注释**：使用等号线分隔不同模块
```cpp
// ==================== 词法单元类型枚举 ====================
```

3. **类和结构体注释**：说明类的职责
```cpp
// 负责将输入字符串分解为词法单元序列
class Lexer {
```

4. **成员变量注释**：行内注释说明用途
```cpp
string input;           // 输入字符串
size_t pos;             // 当前位置
int currentLine;        // 当前行号
```

5. **函数注释**：说明函数功能和对应的文法规则
```cpp
// program -> decls compoundstmt
void parseProgram() {
```

### 中文标点规范

按照要求，在中文注释中使用中文逗号"，"：
```cpp
// 如果是整数值，输出一位小数
// 否则按原样输出
```

---

## 10. 单头文件模块化设计

### 技术说明

整个Translation Schema的实现都包含在单个头文件`TranslationSchema.h`中，符合评测系统要求，同时保持了良好的模块化结构。

### 设计优势

1. **编译简单**：只需包含一个头文件即可使用
2. **依赖清晰**：所有代码集中管理，无外部依赖
3. **评测友好**：符合OJ系统的提交要求

### 文件结构

```cpp
// TranslationSchema.h 文件结构

// 1. 头文件包含
#include <cstdio>
#include <cstring>
// ...

// 2. 标准输入函数（题目要求不修改）
void read_prog(string& prog) { ... }

// 3. 词法单元定义
enum TokenType { ... };
struct Token { ... };

// 4. 值和符号表定义
struct Value { ... };
struct Symbol { ... };

// 5. 词法分析器类
class Lexer { ... };

// 6. 语法分析器和解释器类
class Parser { ... };

// 7. 主分析函数
void Analysis() { ... }
```

### 模块间依赖关系

```
Analysis()
    |
    v
Parser类 -----> Lexer类
    |              |
    v              v
Symbol/Value   Token/TokenType
```

---

## 附录：项目文件清单

```
lab_4/
├── TranslationSchema.h    # 主实现文件
├── main.cpp               # 测试程序入口
├── innovations.md         # 实践亮点列表
├── supp.md               # 本补充文档
└── test_scripts/         # 自动化测试脚本
    ├── run_tests.sh      # 测试运行脚本
    ├── test1.in          # 测试输入1
    ├── test1.out         # 期望输出1
    ├── test2.in          # 测试输入2
    ├── test2.out         # 期望输出2
    └── ...
```
