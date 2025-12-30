#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <stack>
#include <algorithm>
using namespace std;

/* 不要修改这个标准输入函数 */
void read_prog(string& prog)
{
    char c;
    while(scanf("%c",&c)!=EOF){
        prog += c;
    }
}

// ============================================================
// Token类：表示词法单元
// 功能：封装词法分析产生的每个token的信息
// 包含：类型（type）、值（value）、行号（line）
// ============================================================
class Token {
public:
    string type;    // token类型，如ID、NUM、关键字（if、while等）、运算符等
    string value;   // token的实际值，如变量名、数字值等
    int line;       // token所在行号，仅计算非空行，用于错误定位

    // 构造函数：初始化token的三个属性
    Token(const string& t = "", const string& v = "", int l = 1)
        : type(t), value(v), line(l) {}
};

// ============================================================
// Lexer类：词法分析器
// 功能：将输入的程序字符串分解为token序列
// 特点：
//   1. 按行处理输入，便于行号统计
//   2. 仅统计非空行，使行号与用户视角一致
//   3. 支持双字符运算符（<=、>=、==）的识别
// ============================================================
class Lexer {
private:
    vector<string> lines;       // 按行分割的输入，每个元素是一行
    int currentLineIdx;         // 当前处理的行索引（从0开始）
    size_t posInLine;           // 当前行内的字符位置
    int contentLineNumber;      // 当前内容行号（仅计非空行）

public:
    // 构造函数：初始化词法分析器，按行分割输入
    Lexer(const string& prog) : currentLineIdx(0), posInLine(0), contentLineNumber(0) {
        // 使用stringstream按行分割输入字符串
        stringstream ss(prog);
        string line;
        while (getline(ss, line)) {
            lines.push_back(line);
        }
    }

    // 检查当前行是否为空行（只包含空白字符）
    // 空行不计入行号统计，使错误提示更符合用户直觉
    bool isEmptyLine(const string& line) {
        for (char c : line) {
            // 非空白字符则不是空行
            if (c != ' ' && c != '\t' && c != '\r') {
                return false;
            }
        }
        return true;
    }

    // 词法分析主函数：将输入转换为token序列
    // 返回：包含所有token的vector，以$结束符结尾
    vector<Token> tokenize() {
        vector<Token> tokens;  // 存储所有识别出的token

        // 遍历所有行
        while (currentLineIdx < (int)lines.size()) {
            string& line = lines[currentLineIdx];

            // 跳过空行，不增加行号计数
            if (isEmptyLine(line)) {
                currentLineIdx++;
                posInLine = 0;
                continue;
            }

            // 非空行，增加内容行号（用户可见的行号）
            contentLineNumber++;

            // 处理当前行的所有token
            posInLine = 0;
            while (posInLine < line.size()) {
                // 跳过行内空白字符（空格和制表符）
                while (posInLine < line.size() &&
                       (line[posInLine] == ' ' || line[posInLine] == '\t')) {
                    posInLine++;
                }

                // 行末检查
                if (posInLine >= line.size()) break;

                // 优先识别双字符运算符（<=、>=、==）
                // 必须在单字符识别之前检查，避免<被错误识别
                if (posInLine + 1 < line.size()) {
                    string twoChar = line.substr(posInLine, 2);
                    if (twoChar == "<=" || twoChar == ">=" || twoChar == "==") {
                        tokens.push_back(Token(twoChar, twoChar, contentLineNumber));
                        posInLine += 2;
                        continue;
                    }
                }

                // 识别单字符符号：括号、分号、运算符等
                char c = line[posInLine];
                if (c == '{' || c == '}' || c == '(' || c == ')' ||
                    c == ';' || c == '=' || c == '<' || c == '>' ||
                    c == '+' || c == '-' || c == '*' || c == '/') {
                    string s(1, c);  // 将字符转为字符串
                    tokens.push_back(Token(s, s, contentLineNumber));
                    posInLine++;
                    continue;
                }

                // 识别标识符和关键字
                // 标识符以字母或下划线开头
                if (isalpha(c) || c == '_') {
                    string word;
                    // 读取完整的标识符（字母、数字、下划线）
                    while (posInLine < line.size() &&
                           (isalnum(line[posInLine]) || line[posInLine] == '_')) {
                        word += line[posInLine];
                        posInLine++;
                    }
                    // 检查是否为保留关键字
                    if (word == "if" || word == "then" || word == "else" || word == "while") {
                        tokens.push_back(Token(word, word, contentLineNumber));
                    } else if (word == "ID" || word == "NUM") {
                        // ID和NUM在测试输入中作为特殊标记直接出现
                        tokens.push_back(Token(word, word, contentLineNumber));
                    } else {
                        // 其他标识符统一归类为ID类型
                        tokens.push_back(Token("ID", word, contentLineNumber));
                    }
                    continue;
                }

                // 识别数字常量
                if (isdigit(c)) {
                    string num;
                    // 读取完整的数字
                    while (posInLine < line.size() && isdigit(line[posInLine])) {
                        num += line[posInLine];
                        posInLine++;
                    }
                    tokens.push_back(Token("NUM", num, contentLineNumber));
                    continue;
                }

                // 跳过其他未识别字符
                posInLine++;
            }

            // 处理完当前行，移动到下一行
            currentLineIdx++;
            posInLine = 0;
        }

        // 添加输入结束标记$，用于语法分析的终止判断
        tokens.push_back(Token("$", "$", contentLineNumber));
        return tokens;
    }
};

// ============================================================
// Grammar类：文法定义类
// 功能：
//   1. 存储文法的产生式
//   2. 自动计算FIRST集合和FOLLOW集合
// 文法采用实验指定的C语言子集文法
// ============================================================
class Grammar {
public:
    // 产生式映射：非终结符 -> 产生式体列表
    // 每个非终结符可以有多个产生式（用于选择）
    map<string, vector<vector<string>>> productions;

    // FIRST集合：记录每个符号可能推导出的首终结符
    map<string, set<string>> firstSets;

    // FOLLOW集合：记录每个非终结符后面可能跟随的终结符
    map<string, set<string>> followSets;

    // 非终结符集合
    set<string> nonTerminals;

    // 终结符集合
    set<string> terminals;

    // 构造函数：初始化文法并计算FIRST/FOLLOW集合
    Grammar() {
        initGrammar();       // 初始化产生式
        computeFirstSets();  // 计算FIRST集合
        computeFollowSets(); // 计算FOLLOW集合
    }

private:
    // 初始化文法产生式
    // 按照实验要求定义C语言子集的文法规则
    void initGrammar() {
        // 定义所有非终结符
        nonTerminals = {"program", "stmt", "compoundstmt", "stmts", "ifstmt",
                        "whilestmt", "assgstmt", "boolexpr", "boolop",
                        "arithexpr", "arithexprprime", "multexpr", "multexprprime",
                        "simpleexpr"};

        // 定义所有终结符，E表示空串（epsilon）
        terminals = {"{", "}", "if", "(", ")", "then", "else", "while",
                     "ID", "=", ";", "<", ">", "<=", ">=", "==",
                     "+", "-", "*", "/", "NUM", "E", "$"};

        // ==================== 产生式定义 ====================

        // program -> compoundstmt
        // 程序由复合语句组成
        productions["program"] = {{"compoundstmt"}};

        // stmt -> ifstmt | whilestmt | assgstmt | compoundstmt
        // 语句可以是if语句、while语句、赋值语句或复合语句
        productions["stmt"] = {{"ifstmt"}, {"whilestmt"}, {"assgstmt"}, {"compoundstmt"}};

        // compoundstmt -> { stmts }
        // 复合语句由花括号包围的语句序列组成
        productions["compoundstmt"] = {{"{", "stmts", "}"}};

        // stmts -> stmt stmts | E
        // 语句序列可以是一个语句后跟更多语句，或者为空
        productions["stmts"] = {{"stmt", "stmts"}, {"E"}};

        // ifstmt -> if ( boolexpr ) then stmt else stmt
        // if语句的完整形式，包含then和else分支
        productions["ifstmt"] = {{"if", "(", "boolexpr", ")", "then", "stmt", "else", "stmt"}};

        // whilestmt -> while ( boolexpr ) stmt
        // while循环语句
        productions["whilestmt"] = {{"while", "(", "boolexpr", ")", "stmt"}};

        // assgstmt -> ID = arithexpr ;
        // 赋值语句：标识符 = 算术表达式 ;
        productions["assgstmt"] = {{"ID", "=", "arithexpr", ";"}};

        // boolexpr -> arithexpr boolop arithexpr
        // 布尔表达式由两个算术表达式和一个比较运算符组成
        productions["boolexpr"] = {{"arithexpr", "boolop", "arithexpr"}};

        // boolop -> < | > | <= | >= | ==
        // 比较运算符
        productions["boolop"] = {{"<"}, {">"}, {"<="}, {">="}, {"=="}};

        // arithexpr -> multexpr arithexprprime
        // 算术表达式由乘法表达式和算术表达式后缀组成
        productions["arithexpr"] = {{"multexpr", "arithexprprime"}};

        // arithexprprime -> + multexpr arithexprprime | - multexpr arithexprprime | E
        // 算术表达式后缀处理加减运算，使用左递归消除
        productions["arithexprprime"] = {{"+", "multexpr", "arithexprprime"},
                                         {"-", "multexpr", "arithexprprime"},
                                         {"E"}};

        // multexpr -> simpleexpr multexprprime
        // 乘法表达式由简单表达式和乘法表达式后缀组成
        productions["multexpr"] = {{"simpleexpr", "multexprprime"}};

        // multexprprime -> * simpleexpr multexprprime | / simpleexpr multexprprime | E
        // 乘法表达式后缀处理乘除运算
        productions["multexprprime"] = {{"*", "simpleexpr", "multexprprime"},
                                        {"/", "simpleexpr", "multexprprime"},
                                        {"E"}};

        // simpleexpr -> ID | NUM | ( arithexpr )
        // 简单表达式：标识符、数字或括号表达式
        productions["simpleexpr"] = {{"ID"}, {"NUM"}, {"(", "arithexpr", ")"}};
    }

    // 计算单个符号的FIRST集合（递归算法）
    // 参数：symbol - 要计算FIRST集合的符号
    // 返回：该符号的FIRST集合
    set<string> computeFirst(const string& symbol) {
        set<string> result;

        // 规则1：如果是终结符，FIRST(X) = {X}
        if (terminals.count(symbol)) {
            result.insert(symbol);
            return result;
        }

        // 如果已计算过（避免重复计算和无限递归）
        if (firstSets.count(symbol) && !firstSets[symbol].empty()) {
            return firstSets[symbol];
        }

        // 规则2：对于非终结符，遍历所有产生式
        for (const auto& prod : productions[symbol]) {
            // 处理空产生式 X -> E
            if (prod.size() == 1 && prod[0] == "E") {
                result.insert("E");
                continue;
            }

            // 计算产生式体 Y1Y2...Yk 的FIRST
            bool allHaveEpsilon = true;
            for (const auto& s : prod) {
                set<string> firstOfS = computeFirst(s);

                // 将FIRST(Yi) - {E}加入FIRST(X)
                for (const auto& f : firstOfS) {
                    if (f != "E") result.insert(f);
                }

                // 如果Yi不能推导出空串，停止
                if (firstOfS.find("E") == firstOfS.end()) {
                    allHaveEpsilon = false;
                    break;
                }
            }

            // 如果所有Yi都能推导出E，则将E加入FIRST(X)
            if (allHaveEpsilon) {
                result.insert("E");
            }
        }

        return result;
    }

    // 计算所有符号的FIRST集合
    void computeFirstSets() {
        // 首先初始化终结符的FIRST集合
        for (const auto& t : terminals) {
            firstSets[t].insert(t);
        }

        // 然后计算非终结符的FIRST集合
        for (const auto& nt : nonTerminals) {
            firstSets[nt] = computeFirst(nt);
        }
    }

    // 计算所有非终结符的FOLLOW集合
    // 使用迭代法，直到不再有新元素加入为止
    void computeFollowSets() {
        // 初始化所有FOLLOW集合为空
        for (const auto& nt : nonTerminals) {
            followSets[nt] = set<string>();
        }

        // 规则1：将$加入起始符号的FOLLOW集合
        followSets["program"].insert("$");

        // 迭代计算，直到不动点（不再变化）
        bool changed = true;
        while (changed) {
            changed = false;

            // 遍历所有产生式
            for (const auto& prodPair : productions) {
                const string& lhs = prodPair.first;  // 产生式左部A

                for (const auto& prod : prodPair.second) {
                    // 对于产生式A -> αBβ中的每个非终结符B
                    for (size_t i = 0; i < prod.size(); i++) {
                        const string& B = prod[i];

                        // 只处理非终结符
                        if (nonTerminals.find(B) == nonTerminals.end()) continue;

                        // 计算β的FIRST集合
                        set<string> firstOfBeta;
                        bool betaHasEpsilon = true;

                        for (size_t j = i + 1; j < prod.size(); j++) {
                            const string& beta = prod[j];
                            set<string>& firstBeta = firstSets[beta];

                            // 将FIRST(β) - {E}加入
                            for (const auto& f : firstBeta) {
                                if (f != "E") firstOfBeta.insert(f);
                            }

                            // 如果β不能推导出E，停止
                            if (firstBeta.find("E") == firstBeta.end()) {
                                betaHasEpsilon = false;
                                break;
                            }
                        }

                        // 规则2：将FIRST(β) - {E}加入FOLLOW(B)
                        for (const auto& f : firstOfBeta) {
                            if (followSets[B].find(f) == followSets[B].end()) {
                                followSets[B].insert(f);
                                changed = true;  // 有新元素加入
                            }
                        }

                        // 规则3：如果β可以推导出E或B在产生式末尾
                        // 将FOLLOW(A)加入FOLLOW(B)
                        if (betaHasEpsilon || i == prod.size() - 1) {
                            for (const auto& f : followSets[lhs]) {
                                if (followSets[B].find(f) == followSets[B].end()) {
                                    followSets[B].insert(f);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
};

// ============================================================
// ParseTable类：LL(1)分析表
// 功能：根据文法的FIRST和FOLLOW集合构建预测分析表
// 表结构：table[非终结符][终结符] = 产生式索引
// ============================================================
class ParseTable {
public:
    // 分析表：二维map实现，支持O(log n)的查询
    map<string, map<string, int>> table;

    // 对文法的引用，用于获取产生式
    Grammar& grammar;

    // 构造函数：根据文法构建分析表
    ParseTable(Grammar& g) : grammar(g) {
        buildTable();
    }

private:
    // 构建LL(1)分析表
    // 算法：对于每个产生式A -> α
    //   1. 对于FIRST(α)中的每个终结符a，将产生式加入M[A,a]
    //   2. 如果E在FIRST(α)中，对于FOLLOW(A)中的每个终结符b，将产生式加入M[A,b]
    void buildTable() {
        // 遍历所有产生式
        for (const auto& prodPair : grammar.productions) {
            const string& A = prodPair.first;  // 非终结符
            const vector<vector<string>>& prods = prodPair.second;  // 产生式列表

            for (size_t i = 0; i < prods.size(); i++) {
                const vector<string>& alpha = prods[i];  // 产生式体

                // 计算产生式体α的FIRST集合
                set<string> firstAlpha;
                bool hasEpsilon = true;

                for (const auto& symbol : alpha) {
                    // 遇到空产生式
                    if (symbol == "E") {
                        break;
                    }

                    set<string>& firstSym = grammar.firstSets[symbol];
                    // 将FIRST(symbol) - {E}加入firstAlpha
                    for (const auto& f : firstSym) {
                        if (f != "E") firstAlpha.insert(f);
                    }

                    // 如果symbol不能推导出E，停止
                    if (firstSym.find("E") == firstSym.end()) {
                        hasEpsilon = false;
                        break;
                    }
                }

                // 规则1：对于FIRST(α)中的每个终结符a，M[A,a] = i
                for (const auto& a : firstAlpha) {
                    table[A][a] = i;
                }

                // 规则2：如果α可以推导出E，对于FOLLOW(A)中的每个终结符b
                if (hasEpsilon || (alpha.size() == 1 && alpha[0] == "E")) {
                    for (const auto& b : grammar.followSets[A]) {
                        // 避免覆盖已有表项（处理冲突）
                        if (table[A].find(b) == table[A].end()) {
                            table[A][b] = i;
                        }
                    }
                }
            }
        }
    }

public:
    // 根据非终结符和当前输入获取对应的产生式
    // 返回：产生式体（符号序列），空表示错误
    vector<string> getProduction(const string& nonTerminal, const string& terminal) {
        if (table.count(nonTerminal) && table[nonTerminal].count(terminal)) {
            int idx = table[nonTerminal][terminal];
            return grammar.productions[nonTerminal][idx];
        }
        return vector<string>();  // 空向量表示错误
    }

    // 检查分析表中是否存在对应表项
    bool hasEntry(const string& nonTerminal, const string& terminal) {
        return table.count(nonTerminal) && table[nonTerminal].count(terminal);
    }
};

// ============================================================
// TreeNode类：语法树节点
// 功能：表示语法分析生成的语法树的节点
// 支持：子节点管理、递归输出（带缩进）
// ============================================================
class TreeNode {
public:
    string symbol;              // 节点符号（终结符或非终结符名称）
    vector<TreeNode*> children; // 子节点列表

    // 构造函数
    TreeNode(const string& s) : symbol(s) {}

    // 析构函数：递归释放所有子节点
    ~TreeNode() {
        for (auto child : children) {
            delete child;
        }
    }

    // 添加子节点
    void addChild(TreeNode* child) {
        children.push_back(child);
    }

    // 输出语法树（前序遍历，使用tab缩进）
    // 参数：depth - 当前节点深度，用于控制缩进
    void print(int depth = 0) const {
        // 输出缩进：每层一个tab
        for (int i = 0; i < depth; i++) {
            cout << "\t";
        }
        // 输出节点符号
        cout << symbol << endl;

        // 递归输出所有子节点
        for (const auto& child : children) {
            child->print(depth + 1);
        }
    }
};

// ============================================================
// LLParser类：LL(1)语法分析器
// 功能：
//   1. 调用词法分析器获取token序列
//   2. 使用LL(1)分析表进行语法分析
//   3. 构建并输出语法树
//   4. 支持错误检测和恢复
// ============================================================
class LLParser {
private:
    Grammar grammar;            // 文法定义
    ParseTable parseTable;      // LL(1)分析表
    vector<Token> tokens;       // token序列
    size_t currentPos;          // 当前token位置
    vector<string> errors;      // 错误信息列表
    int lastErrorLine;          // 上次报错的行号，避免重复报错

public:
    // 构造函数：初始化文法和分析表
    LLParser() : parseTable(grammar), currentPos(0), lastErrorLine(-1) {}

    // 解析入口函数
    // 参数：prog - 输入的程序字符串
    // 返回：语法树的根节点
    TreeNode* parse(const string& prog) {
        // 第一步：词法分析，获取token序列
        Lexer lexer(prog);
        tokens = lexer.tokenize();
        currentPos = 0;
        errors.clear();
        lastErrorLine = -1;

        // 第二步：语法分析，从起始符号program开始
        TreeNode* root = parseNonTerminal("program");

        // 第三步：输出错误信息（在语法树之前）
        for (const auto& err : errors) {
            cout << err << endl;
        }

        return root;
    }

private:
    // 获取当前待处理的token
    Token& currentToken() {
        return tokens[currentPos];
    }

    // 获取前一个token的行号（用于错误报告）
    // 因为缺少的符号应该出现在前一个token之后
    int getPrevTokenLine() {
        if (currentPos > 0) {
            return tokens[currentPos - 1].line;
        }
        return tokens[currentPos].line;
    }

    // 前进到下一个token
    void advance() {
        if (currentPos < tokens.size() - 1) {
            currentPos++;
        }
    }

    // 递归下降解析非终结符
    // 参数：nonTerminal - 要解析的非终结符
    // 返回：对应的语法树节点
    TreeNode* parseNonTerminal(const string& nonTerminal) {
        // 创建当前非终结符的节点
        TreeNode* node = new TreeNode(nonTerminal);

        // 获取当前输入符号（向前看符号）
        string lookahead = currentToken().type;

        // 在分析表中查找对应的产生式
        if (!parseTable.hasEntry(nonTerminal, lookahead)) {
            // 分析表中无对应项，进行错误处理
            handleError(nonTerminal, lookahead, node);
            return node;
        }

        // 获取要使用的产生式
        vector<string> production = parseTable.getProduction(nonTerminal, lookahead);

        // 处理空产生式（E表示epsilon）
        if (production.size() == 1 && production[0] == "E") {
            node->addChild(new TreeNode("E"));
            return node;
        }

        // 按顺序处理产生式中的每个符号
        for (const auto& symbol : production) {
            if (grammar.nonTerminals.count(symbol)) {
                // 符号是非终结符：递归解析
                TreeNode* child = parseNonTerminal(symbol);
                node->addChild(child);
            } else {
                // 符号是终结符：尝试匹配
                if (currentToken().type == symbol) {
                    // 匹配成功，添加终结符节点并前进
                    node->addChild(new TreeNode(symbol));
                    advance();
                } else {
                    // 匹配失败：缺少终结符，进行错误处理
                    handleMissingTerminal(symbol, node);
                }
            }
        }

        return node;
    }

    // 处理缺少终结符的错误（插入恢复策略）
    // 参数：expected - 期望的终结符
    //       parent - 父节点，用于添加恢复的节点
    void handleMissingTerminal(const string& expected, TreeNode* parent) {
        // 使用前一个token的行号报错
        int line = getPrevTokenLine();

        // 避免在同一行重复报错
        if (line != lastErrorLine || errors.empty()) {
            // 生成错误信息，格式：语法错误,第X行,缺少"Y"
            string errorMsg = "语法错误,第" + to_string(line) + "行,缺少\"" + expected + "\"";
            errors.push_back(errorMsg);
            lastErrorLine = line;
        }

        // 错误恢复：插入缺失的终结符节点
        // 这样可以继续解析后续内容
        parent->addChild(new TreeNode(expected));
    }

    // 处理分析表无对应项的错误（同步恢复策略）
    // 参数：nonTerminal - 当前要解析的非终结符
    //       lookahead - 当前输入符号
    //       node - 当前节点
    void handleError(const string& nonTerminal, const string& lookahead, TreeNode* node) {
        // 策略1：如果当前输入在FOLLOW集合中，使用空产生式恢复
        // 这意味着该非终结符推导为空，继续解析后续部分
        if (grammar.followSets[nonTerminal].count(lookahead)) {
            node->addChild(new TreeNode("E"));
            return;
        }

        // 策略2：对于stmts遇到}，说明语句序列结束
        if (nonTerminal == "stmts" && lookahead == "}") {
            node->addChild(new TreeNode("E"));
            return;
        }

        // 策略3：检查该非终结符是否可以为空
        bool canBeEmpty = false;
        for (const auto& prod : grammar.productions[nonTerminal]) {
            if (prod.size() == 1 && prod[0] == "E") {
                canBeEmpty = true;
                break;
            }
        }
        if (canBeEmpty) {
            node->addChild(new TreeNode("E"));
            return;
        }

        // 策略4：跳过当前token，重新尝试解析
        advance();

        // 重新尝试解析当前非终结符
        if (currentPos < tokens.size() - 1) {
            TreeNode* retry = parseNonTerminal(nonTerminal);
            // 将重试得到的子节点转移到当前节点
            for (auto child : retry->children) {
                node->addChild(child);
            }
            retry->children.clear();
            delete retry;
        }
    }
};

// ============================================================
// Analysis函数：主分析入口
// 功能：读取输入，创建解析器，输出语法树
// ============================================================
void Analysis()
{
    string prog;
    read_prog(prog);  // 读取输入程序

    /********* Begin *********/

    // 创建LL(1)语法分析器实例
    LLParser parser;

    // 解析程序，构建语法树
    TreeNode* root = parser.parse(prog);

    // 输出语法树（使用tab缩进表示层级）
    if (root) {
        root->print();
        delete root;  // 释放内存
    }

    /********* End *********/
}
