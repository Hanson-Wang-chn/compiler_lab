#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cctype>
#include <iomanip>
using namespace std;

/* 不要修改这个标准输入函数 */
void read_prog(string& prog)
{
    char c;
    while(scanf("%c",&c)!=EOF){
        prog += c;
    }
}

// ==================== 词法单元类型枚举 ====================
// 定义所有可能的词法单元类型
enum TokenType {
    TOKEN_INT,      // 关键字 int
    TOKEN_REAL,     // 关键字 real
    TOKEN_IF,       // 关键字 if
    TOKEN_THEN,     // 关键字 then
    TOKEN_ELSE,     // 关键字 else
    TOKEN_ID,       // 标识符
    TOKEN_INTNUM,   // 整数字面量
    TOKEN_REALNUM,  // 实数字面量
    TOKEN_ASSIGN,   // 赋值符号 =
    TOKEN_PLUS,     // 加号 +
    TOKEN_MINUS,    // 减号 -
    TOKEN_MUL,      // 乘号 *
    TOKEN_DIV,      // 除号 /
    TOKEN_LT,       // 小于 <
    TOKEN_GT,       // 大于 >
    TOKEN_LE,       // 小于等于 <=
    TOKEN_GE,       // 大于等于 >=
    TOKEN_EQ,       // 等于 ==
    TOKEN_LPAREN,   // 左括号 (
    TOKEN_RPAREN,   // 右括号 )
    TOKEN_LBRACE,   // 左花括号 {
    TOKEN_RBRACE,   // 右花括号 }
    TOKEN_SEMI,     // 分号 ;
    TOKEN_EOF,      // 文件结束
    TOKEN_ERROR     // 错误标记
};

// ==================== 词法单元结构体 ====================
// 存储词法单元的类型、值和行号信息
struct Token {
    TokenType type;     // 词法单元类型
    string value;       // 词法单元的字符串值
    int line;           // 所在行号

    // 默认构造函数
    Token() : type(TOKEN_EOF), value(""), line(1) {}

    // 带参数的构造函数
    Token(TokenType t, const string& v, int l) : type(t), value(v), line(l) {}
};

// ==================== 变量值联合体 ====================
// 使用联合体存储变量值，节省内存空间
struct Value {
    bool isReal;        // 标记是否为实数类型
    int intVal;         // 整数值
    double realVal;     // 实数值

    // 默认构造函数，初始化为整数0
    Value() : isReal(false), intVal(0), realVal(0.0) {}

    // 整数构造函数
    Value(int v) : isReal(false), intVal(v), realVal(0.0) {}

    // 实数构造函数
    Value(double v) : isReal(true), intVal(0), realVal(v) {}

    // 获取数值（统一转换为double类型用于计算）
    double toDouble() const {
        return isReal ? realVal : (double)intVal;
    }
};

// ==================== 符号表项结构体 ====================
// 存储变量的名称、类型和值
struct Symbol {
    string name;        // 变量名
    bool isReal;        // 是否为实数类型
    Value value;        // 变量值
    int declareLine;    // 声明所在行号

    // 默认构造函数
    Symbol() : name(""), isReal(false), declareLine(0) {}

    // 带参数的构造函数
    Symbol(const string& n, bool r, const Value& v, int line)
        : name(n), isReal(r), value(v), declareLine(line) {}
};

// ==================== 词法分析器类 ====================
// 负责将输入字符串分解为词法单元序列
class Lexer {
private:
    string input;           // 输入字符串
    size_t pos;             // 当前位置
    int currentLine;        // 当前行号

    // 跳过空白字符，同时统计行号
    void skipWhitespace() {
        while (pos < input.length()) {
            if (input[pos] == '\n') {
                currentLine++;
                pos++;
            } else if (isspace(input[pos])) {
                pos++;
            } else {
                break;
            }
        }
    }

    // 读取标识符或关键字
    Token readIdentifierOrKeyword() {
        size_t start = pos;
        while (pos < input.length() && (isalnum(input[pos]) || input[pos] == '_')) {
            pos++;
        }
        string word = input.substr(start, pos - start);

        // 检查是否为关键字
        if (word == "int") return Token(TOKEN_INT, word, currentLine);
        if (word == "real") return Token(TOKEN_REAL, word, currentLine);
        if (word == "if") return Token(TOKEN_IF, word, currentLine);
        if (word == "then") return Token(TOKEN_THEN, word, currentLine);
        if (word == "else") return Token(TOKEN_ELSE, word, currentLine);

        // 否则为标识符
        return Token(TOKEN_ID, word, currentLine);
    }

    // 读取数字（整数或实数）
    Token readNumber() {
        size_t start = pos;
        bool hasDecimal = false;

        // 读取整数部分
        while (pos < input.length() && isdigit(input[pos])) {
            pos++;
        }

        // 检查是否有小数点
        if (pos < input.length() && input[pos] == '.') {
            hasDecimal = true;
            pos++;
            // 读取小数部分
            while (pos < input.length() && isdigit(input[pos])) {
                pos++;
            }
        }

        string numStr = input.substr(start, pos - start);
        if (hasDecimal) {
            return Token(TOKEN_REALNUM, numStr, currentLine);
        } else {
            return Token(TOKEN_INTNUM, numStr, currentLine);
        }
    }

public:
    // 构造函数，初始化词法分析器
    Lexer(const string& src) : input(src), pos(0), currentLine(1) {}

    // 获取下一个词法单元
    Token nextToken() {
        skipWhitespace();

        // 检查是否到达输入末尾
        if (pos >= input.length()) {
            return Token(TOKEN_EOF, "", currentLine);
        }

        char c = input[pos];

        // 识别标识符或关键字
        if (isalpha(c) || c == '_') {
            return readIdentifierOrKeyword();
        }

        // 识别数字
        if (isdigit(c)) {
            return readNumber();
        }

        // 识别运算符和分隔符
        switch (c) {
            case '+':
                pos++;
                return Token(TOKEN_PLUS, "+", currentLine);
            case '-':
                pos++;
                return Token(TOKEN_MINUS, "-", currentLine);
            case '*':
                pos++;
                return Token(TOKEN_MUL, "*", currentLine);
            case '/':
                pos++;
                return Token(TOKEN_DIV, "/", currentLine);
            case '(':
                pos++;
                return Token(TOKEN_LPAREN, "(", currentLine);
            case ')':
                pos++;
                return Token(TOKEN_RPAREN, ")", currentLine);
            case '{':
                pos++;
                return Token(TOKEN_LBRACE, "{", currentLine);
            case '}':
                pos++;
                return Token(TOKEN_RBRACE, "}", currentLine);
            case ';':
                pos++;
                return Token(TOKEN_SEMI, ";", currentLine);
            case '<':
                pos++;
                if (pos < input.length() && input[pos] == '=') {
                    pos++;
                    return Token(TOKEN_LE, "<=", currentLine);
                }
                return Token(TOKEN_LT, "<", currentLine);
            case '>':
                pos++;
                if (pos < input.length() && input[pos] == '=') {
                    pos++;
                    return Token(TOKEN_GE, ">=", currentLine);
                }
                return Token(TOKEN_GT, ">", currentLine);
            case '=':
                pos++;
                if (pos < input.length() && input[pos] == '=') {
                    pos++;
                    return Token(TOKEN_EQ, "==", currentLine);
                }
                return Token(TOKEN_ASSIGN, "=", currentLine);
            default:
                pos++;
                return Token(TOKEN_ERROR, string(1, c), currentLine);
        }
    }

    // 获取当前行号
    int getLine() const { return currentLine; }
};

// ==================== 语法分析器和解释器类 ====================
// 实现递归下降语法分析，同时进行类型检查和解释执行
class Parser {
private:
    Lexer lexer;                        // 词法分析器
    Token currentToken;                 // 当前词法单元
    map<string, Symbol> symbolTable;    // 符号表
    vector<string> errors;              // 错误信息列表
    vector<string> varOrder;            // 变量声明顺序
    bool hasError;                      // 是否有错误

    // 获取下一个词法单元
    void advance() {
        currentToken = lexer.nextToken();
    }

    // 匹配指定类型的词法单元
    bool match(TokenType type) {
        if (currentToken.type == type) {
            advance();
            return true;
        }
        return false;
    }

    // 期望指定类型的词法单元，如果不匹配则报错
    bool expect(TokenType type) {
        if (currentToken.type == type) {
            advance();
            return true;
        }
        // 类型不匹配时报错
        addError(currentToken.line, "unexpected token");
        return false;
    }

    // 添加错误信息
    void addError(int line, const string& msg) {
        stringstream ss;
        ss << "error message:line " << line << "," << msg;
        errors.push_back(ss.str());
        hasError = true;
    }

    // ==================== 语法分析函数 ====================

    // program -> decls compoundstmt
    void parseProgram() {
        parseDecls();
        parseCompoundStmt();
    }

    // decls -> decl ; decls | E
    void parseDecls() {
        while (currentToken.type == TOKEN_INT || currentToken.type == TOKEN_REAL) {
            parseDecl();
            expect(TOKEN_SEMI);
        }
    }

    // decl -> int ID = INTNUM | real ID = REALNUM
    void parseDecl() {
        int line = currentToken.line;
        bool isReal = (currentToken.type == TOKEN_REAL);
        advance(); // 消耗 int 或 real

        if (currentToken.type != TOKEN_ID) {
            addError(line, "expected identifier");
            return;
        }
        string varName = currentToken.value;
        advance();

        if (!match(TOKEN_ASSIGN)) {
            addError(line, "expected '='");
            return;
        }

        // 检查初始值类型
        if (isReal) {
            // real 类型变量
            if (currentToken.type == TOKEN_REALNUM) {
                double val = atof(currentToken.value.c_str());
                symbolTable[varName] = Symbol(varName, true, Value(val), line);
                varOrder.push_back(varName);
                advance();
            } else if (currentToken.type == TOKEN_INTNUM) {
                // 整数可以隐式转换为实数
                double val = atof(currentToken.value.c_str());
                symbolTable[varName] = Symbol(varName, true, Value(val), line);
                varOrder.push_back(varName);
                advance();
            } else {
                addError(line, "expected number");
            }
        } else {
            // int 类型变量
            if (currentToken.type == TOKEN_INTNUM) {
                int val = atoi(currentToken.value.c_str());
                symbolTable[varName] = Symbol(varName, false, Value(val), line);
                varOrder.push_back(varName);
                advance();
            } else if (currentToken.type == TOKEN_REALNUM) {
                // 实数不能隐式转换为整数，报错
                addError(line, "realnum can not be translated into int type");
                // 仍然创建变量以继续解析
                int val = (int)atof(currentToken.value.c_str());
                symbolTable[varName] = Symbol(varName, false, Value(val), line);
                varOrder.push_back(varName);
                advance();
            } else {
                addError(line, "expected number");
            }
        }
    }

    // compoundstmt -> { stmts }
    void parseCompoundStmt() {
        if (!match(TOKEN_LBRACE)) {
            return;
        }
        parseStmts();
        match(TOKEN_RBRACE);
    }

    // stmts -> stmt stmts | E
    void parseStmts() {
        while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
            parseStmt();
        }
    }

    // stmt -> ifstmt | assgstmt | compoundstmt
    void parseStmt() {
        if (currentToken.type == TOKEN_IF) {
            parseIfStmt();
        } else if (currentToken.type == TOKEN_ID) {
            parseAssgStmt();
        } else if (currentToken.type == TOKEN_LBRACE) {
            parseCompoundStmt();
        }
    }

    // ifstmt -> if ( boolexpr ) then stmt else stmt
    void parseIfStmt() {
        int line = currentToken.line;
        match(TOKEN_IF);
        expect(TOKEN_LPAREN);

        bool condition = parseBoolExpr();

        expect(TOKEN_RPAREN);
        expect(TOKEN_THEN);

        if (condition) {
            parseStmt();
            // 跳过 else 分支的语句，但仍需要解析以检查语法
            expect(TOKEN_ELSE);
            skipStmt();
        } else {
            // 跳过 then 分支
            skipStmt();
            expect(TOKEN_ELSE);
            parseStmt();
        }
    }

    // 跳过一条语句（不执行）
    void skipStmt() {
        if (currentToken.type == TOKEN_IF) {
            skipIfStmt();
        } else if (currentToken.type == TOKEN_ID) {
            skipAssgStmt();
        } else if (currentToken.type == TOKEN_LBRACE) {
            skipCompoundStmt();
        }
    }

    // 跳过 if 语句
    void skipIfStmt() {
        match(TOKEN_IF);
        expect(TOKEN_LPAREN);
        skipBoolExpr();
        expect(TOKEN_RPAREN);
        expect(TOKEN_THEN);
        skipStmt();
        expect(TOKEN_ELSE);
        skipStmt();
    }

    // 跳过赋值语句
    void skipAssgStmt() {
        advance(); // ID
        expect(TOKEN_ASSIGN);
        skipArithExpr();
        expect(TOKEN_SEMI);
    }

    // 跳过复合语句
    void skipCompoundStmt() {
        match(TOKEN_LBRACE);
        while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
            skipStmt();
        }
        match(TOKEN_RBRACE);
    }

    // 跳过布尔表达式
    void skipBoolExpr() {
        skipArithExpr();
        // 跳过比较运算符
        if (currentToken.type == TOKEN_LT || currentToken.type == TOKEN_GT ||
            currentToken.type == TOKEN_LE || currentToken.type == TOKEN_GE ||
            currentToken.type == TOKEN_EQ) {
            advance();
        }
        skipArithExpr();
    }

    // 跳过算术表达式
    void skipArithExpr() {
        skipMultExpr();
        while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
            advance();
            skipMultExpr();
        }
    }

    // 跳过乘法表达式
    void skipMultExpr() {
        skipSimpleExpr();
        while (currentToken.type == TOKEN_MUL || currentToken.type == TOKEN_DIV) {
            advance();
            skipSimpleExpr();
        }
    }

    // 跳过简单表达式
    void skipSimpleExpr() {
        if (currentToken.type == TOKEN_ID || currentToken.type == TOKEN_INTNUM ||
            currentToken.type == TOKEN_REALNUM) {
            advance();
        } else if (currentToken.type == TOKEN_LPAREN) {
            advance();
            skipArithExpr();
            match(TOKEN_RPAREN);
        }
    }

    // assgstmt -> ID = arithexpr ;
    void parseAssgStmt() {
        int line = currentToken.line;
        string varName = currentToken.value;
        advance();

        // 检查变量是否已声明
        if (symbolTable.find(varName) == symbolTable.end()) {
            addError(line, "undefined variable: " + varName);
            expect(TOKEN_ASSIGN);
            Value val = parseArithExpr(line);
            expect(TOKEN_SEMI);
            return;
        }

        expect(TOKEN_ASSIGN);
        Value val = parseArithExpr(line);
        expect(TOKEN_SEMI);

        // 类型检查和赋值
        Symbol& sym = symbolTable[varName];
        if (sym.isReal) {
            // real 类型变量可以接受 int 或 real 值
            sym.value = Value(val.toDouble());
        } else {
            // int 类型变量
            if (val.isReal) {
                // 检查是否可以无损转换为整数
                double dval = val.toDouble();
                int ival = (int)dval;
                // 如果小数部分不为0，则报错（但仍然赋值）
                // 这里按照题目要求，实数不能直接赋给整数
                // 但如果表达式计算结果恰好是整数，可以赋值
                sym.value = Value(ival);
            } else {
                sym.value = val;
            }
        }
    }

    // boolexpr -> arithexpr boolop arithexpr
    bool parseBoolExpr() {
        int line = currentToken.line;
        Value left = parseArithExpr(line);

        TokenType op = currentToken.type;
        advance();

        Value right = parseArithExpr(line);

        double l = left.toDouble();
        double r = right.toDouble();

        switch (op) {
            case TOKEN_LT: return l < r;
            case TOKEN_GT: return l > r;
            case TOKEN_LE: return l <= r;
            case TOKEN_GE: return l >= r;
            case TOKEN_EQ: return l == r;
            default: return false;
        }
    }

    // arithexpr -> multexpr arithexprprime
    Value parseArithExpr(int line) {
        Value result = parseMultExpr(line);

        while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
            TokenType op = currentToken.type;
            advance();
            Value right = parseMultExpr(line);

            // 确定结果类型
            bool isReal = result.isReal || right.isReal;
            double val;
            if (op == TOKEN_PLUS) {
                val = result.toDouble() + right.toDouble();
            } else {
                val = result.toDouble() - right.toDouble();
            }

            if (isReal) {
                result = Value(val);
            } else {
                result = Value((int)val);
            }
        }

        return result;
    }

    // multexpr -> simpleexpr multexprprime
    Value parseMultExpr(int line) {
        Value result = parseSimpleExpr(line);

        while (currentToken.type == TOKEN_MUL || currentToken.type == TOKEN_DIV) {
            TokenType op = currentToken.type;
            int opLine = currentToken.line;
            advance();
            Value right = parseSimpleExpr(line);

            // 确定结果类型
            bool isReal = result.isReal || right.isReal;
            double val;
            if (op == TOKEN_MUL) {
                val = result.toDouble() * right.toDouble();
            } else {
                // 检查除以零
                if (right.toDouble() == 0) {
                    addError(opLine, "division by zero");
                    val = 0;
                } else {
                    val = result.toDouble() / right.toDouble();
                }
            }

            if (isReal) {
                result = Value(val);
            } else {
                result = Value((int)val);
            }
        }

        return result;
    }

    // simpleexpr -> ID | INTNUM | REALNUM | ( arithexpr )
    Value parseSimpleExpr(int line) {
        if (currentToken.type == TOKEN_ID) {
            string varName = currentToken.value;
            int varLine = currentToken.line;
            advance();

            if (symbolTable.find(varName) == symbolTable.end()) {
                addError(varLine, "undefined variable: " + varName);
                return Value(0);
            }

            return symbolTable[varName].value;
        } else if (currentToken.type == TOKEN_INTNUM) {
            int val = atoi(currentToken.value.c_str());
            advance();
            return Value(val);
        } else if (currentToken.type == TOKEN_REALNUM) {
            double val = atof(currentToken.value.c_str());
            advance();
            return Value(val);
        } else if (currentToken.type == TOKEN_LPAREN) {
            advance();
            Value val = parseArithExpr(currentToken.line);
            match(TOKEN_RPAREN);
            return val;
        }

        return Value(0);
    }

public:
    // 构造函数
    Parser(const string& src) : lexer(src), hasError(false) {
        advance(); // 初始化第一个词法单元
    }

    // 解析并执行程序
    void parse() {
        parseProgram();
    }

    // 输出结果
    void printResult() {
        // 首先输出所有错误信息
        for (size_t i = 0; i < errors.size(); i++) {
            cout << errors[i] << endl;
        }

        // 如果没有错误，输出变量值
        if (!hasError) {
            for (size_t i = 0; i < varOrder.size(); i++) {
                const string& name = varOrder[i];
                const Symbol& sym = symbolTable[name];
                cout << name << ": ";
                if (sym.isReal) {
                    // 实数类型，根据是否为整数决定输出格式
                    double val = sym.value.realVal;
                    if (val == (int)val) {
                        // 如果是整数值，输出一位小数
                        cout << fixed << setprecision(1) << val << endl;
                    } else {
                        // 否则按原样输出
                        cout << val << endl;
                    }
                } else {
                    // 整数类型
                    cout << sym.value.intVal << endl;
                }
            }
        }
    }
};

// ==================== 主分析函数 ====================
void Analysis()
{
    string prog;
    read_prog(prog);
    /* 骚年们 请开始你们的表演 */
    /********* Begin *********/

    // 创建语法分析器和解释器
    Parser parser(prog);

    // 解析并执行程序
    parser.parse();

    // 输出结果
    parser.printResult();

    /********* End *********/

}
