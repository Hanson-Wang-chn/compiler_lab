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
#include <queue>
using namespace std;

/* 不要修改这个标准输入函数 */
void read_prog(string& prog)
{
    char c;
    while(scanf("%c",&c)!=EOF){
        prog += c;
    }
}

// ==================== 符号类 ====================
// 表示文法中的终结符或非终结符
class Symbol {
public:
    string name;        // 符号名称
    bool isTerminal;    // 是否为终结符

    Symbol() : name(""), isTerminal(true) {}
    Symbol(const string& n, bool term) : name(n), isTerminal(term) {}

    bool operator==(const Symbol& other) const {
        return name == other.name && isTerminal == other.isTerminal;
    }

    bool operator<(const Symbol& other) const {
        if (name != other.name) return name < other.name;
        return isTerminal < other.isTerminal;
    }

    bool operator!=(const Symbol& other) const {
        return !(*this == other);
    }
};

// ==================== 产生式类 ====================
// 表示一条文法产生式：左部 -> 右部
class Production {
public:
    Symbol left;                // 产生式左部（非终结符）
    vector<Symbol> right;       // 产生式右部（符号序列）
    int id;                     // 产生式编号

    Production() : id(-1) {}
    Production(const Symbol& l, const vector<Symbol>& r, int i)
        : left(l), right(r), id(i) {}

    bool operator==(const Production& other) const {
        return left == other.left && right == other.right;
    }

    bool operator<(const Production& other) const {
        if (left != other.left) return left < other.left;
        return right < other.right;
    }
};

// ==================== LR(1)项目类 ====================
// 表示一个LR(1)项目：[A -> α·β, a]
class LR1Item {
public:
    Production prod;    // 对应的产生式
    int dotPos;         // 点的位置（0表示在最左边）
    Symbol lookahead;   // 向前看符号

    LR1Item() : dotPos(0) {}
    LR1Item(const Production& p, int d, const Symbol& la)
        : prod(p), dotPos(d), lookahead(la) {}

    // 获取点后面的符号，如果点在末尾则返回空符号
    Symbol getSymbolAfterDot() const {
        if (dotPos < (int)prod.right.size() &&
            !(prod.right.size() == 1 && prod.right[0].name == "E")) {
            return prod.right[dotPos];
        }
        return Symbol("", true);  // 空符号表示点在末尾
    }

    // 判断点是否在产生式末尾
    bool isComplete() const {
        // 如果右部是空产生式E，则点位置0就是完成状态
        if (prod.right.size() == 1 && prod.right[0].name == "E") {
            return true;
        }
        return dotPos >= (int)prod.right.size();
    }

    bool operator==(const LR1Item& other) const {
        return prod == other.prod && dotPos == other.dotPos &&
               lookahead == other.lookahead;
    }

    bool operator<(const LR1Item& other) const {
        if (prod < other.prod) return true;
        if (other.prod < prod) return false;
        if (dotPos != other.dotPos) return dotPos < other.dotPos;
        return lookahead < other.lookahead;
    }
};

// ==================== LR(1)状态类 ====================
// 表示LR(1)自动机的一个状态（项目集）
class LR1State {
public:
    set<LR1Item> items;     // 项目集合
    int id;                 // 状态编号

    LR1State() : id(-1) {}

    bool operator==(const LR1State& other) const {
        return items == other.items;
    }

    bool operator<(const LR1State& other) const {
        return items < other.items;
    }
};

// ==================== 词法单元类 ====================
// 表示输入的一个词法单元
class Token {
public:
    string value;   // 词法单元的值
    int line;       // 所在行号

    Token() : value(""), line(1) {}
    Token(const string& v, int l) : value(v), line(l) {}
};

// ==================== LR(1)语法分析器类 ====================
// 核心分析器类，实现完整的LR(1)分析算法
class LRParser {
private:
    // 文法相关数据结构
    vector<Production> productions;         // 所有产生式
    set<Symbol> terminals;                  // 终结符集合
    set<Symbol> nonTerminals;               // 非终结符集合
    Symbol startSymbol;                     // 起始符号
    Symbol augmentedStart;                  // 增广文法起始符号

    // FIRST和FOLLOW集
    map<Symbol, set<Symbol>> firstSet;      // FIRST集
    map<Symbol, set<Symbol>> followSet;     // FOLLOW集

    // LR(1)自动机
    vector<LR1State> states;                // 所有状态
    map<pair<int, Symbol>, int> gotoTable;  // GOTO表

    // ACTION表：状态和终结符 -> 动作
    // 动作编码：正数表示移入状态号，负数表示按产生式规约（-1表示接受）
    map<pair<int, Symbol>, int> actionTable;

    // 词法分析结果
    vector<Token> tokens;                   // 词法单元序列
    int currentToken;                       // 当前词法单元位置
    int lastTokenLine;                      // 上一个词法单元的行号

    // 错误信息
    vector<string> errors;                  // 错误消息列表

    // 推导过程
    vector<string> derivation;              // 最右推导序列

public:
    LRParser() : currentToken(0), lastTokenLine(1) {
        initGrammar();
        computeFirstSets();
        computeFollowSets();
        buildLR1Automaton();
        buildParsingTable();
    }

    // 初始化文法
    void initGrammar() {
        // 定义终结符
        vector<string> termNames = {
            "{", "}", "if", "(", ")", "then", "else", "while",
            "ID", "=", ";", "<", ">", "<=", ">=", "==",
            "+", "-", "*", "/", "NUM", "$"
        };
        for (const string& t : termNames) {
            terminals.insert(Symbol(t, true));
        }

        // 定义非终结符
        vector<string> nonTermNames = {
            "program'", "program", "stmt", "compoundstmt", "stmts",
            "ifstmt", "whilestmt", "assgstmt", "boolexpr", "boolop",
            "arithexpr", "arithexprprime", "multexpr", "multexprprime", "simpleexpr"
        };
        for (const string& nt : nonTermNames) {
            nonTerminals.insert(Symbol(nt, false));
        }

        // 起始符号
        startSymbol = Symbol("program", false);
        augmentedStart = Symbol("program'", false);

        // 定义产生式
        int prodId = 0;

        // 增广产生式：program' -> program
        addProduction("program'", {"program"}, prodId++);

        // program -> compoundstmt
        addProduction("program", {"compoundstmt"}, prodId++);

        // stmt -> ifstmt | whilestmt | assgstmt | compoundstmt
        addProduction("stmt", {"ifstmt"}, prodId++);
        addProduction("stmt", {"whilestmt"}, prodId++);
        addProduction("stmt", {"assgstmt"}, prodId++);
        addProduction("stmt", {"compoundstmt"}, prodId++);

        // compoundstmt -> { stmts }
        addProduction("compoundstmt", {"{", "stmts", "}"}, prodId++);

        // stmts -> stmt stmts | E
        addProduction("stmts", {"stmt", "stmts"}, prodId++);
        addProduction("stmts", {"E"}, prodId++);  // 空产生式

        // ifstmt -> if ( boolexpr ) then stmt else stmt
        addProduction("ifstmt", {"if", "(", "boolexpr", ")", "then", "stmt", "else", "stmt"}, prodId++);

        // whilestmt -> while ( boolexpr ) stmt
        addProduction("whilestmt", {"while", "(", "boolexpr", ")", "stmt"}, prodId++);

        // assgstmt -> ID = arithexpr ;
        addProduction("assgstmt", {"ID", "=", "arithexpr", ";"}, prodId++);

        // boolexpr -> arithexpr boolop arithexpr
        addProduction("boolexpr", {"arithexpr", "boolop", "arithexpr"}, prodId++);

        // boolop -> < | > | <= | >= | ==
        addProduction("boolop", {"<"}, prodId++);
        addProduction("boolop", {">"}, prodId++);
        addProduction("boolop", {"<="}, prodId++);
        addProduction("boolop", {">="}, prodId++);
        addProduction("boolop", {"=="}, prodId++);

        // arithexpr -> multexpr arithexprprime
        addProduction("arithexpr", {"multexpr", "arithexprprime"}, prodId++);

        // arithexprprime -> + multexpr arithexprprime | - multexpr arithexprprime | E
        addProduction("arithexprprime", {"+", "multexpr", "arithexprprime"}, prodId++);
        addProduction("arithexprprime", {"-", "multexpr", "arithexprprime"}, prodId++);
        addProduction("arithexprprime", {"E"}, prodId++);  // 空产生式

        // multexpr -> simpleexpr multexprprime
        addProduction("multexpr", {"simpleexpr", "multexprprime"}, prodId++);

        // multexprprime -> * simpleexpr multexprprime | / simpleexpr multexprprime | E
        addProduction("multexprprime", {"*", "simpleexpr", "multexprprime"}, prodId++);
        addProduction("multexprprime", {"/", "simpleexpr", "multexprprime"}, prodId++);
        addProduction("multexprprime", {"E"}, prodId++);  // 空产生式

        // simpleexpr -> ID | NUM | ( arithexpr )
        addProduction("simpleexpr", {"ID"}, prodId++);
        addProduction("simpleexpr", {"NUM"}, prodId++);
        addProduction("simpleexpr", {"(", "arithexpr", ")"}, prodId++);
    }

    // 添加产生式辅助函数
    void addProduction(const string& left, const vector<string>& right, int id) {
        Symbol leftSym(left, false);
        vector<Symbol> rightSyms;
        for (const string& r : right) {
            bool isTerm = (terminals.find(Symbol(r, true)) != terminals.end()) || r == "E";
            rightSyms.push_back(Symbol(r, isTerm));
        }
        productions.push_back(Production(leftSym, rightSyms, id));
    }

    // 计算FIRST集
    void computeFirstSets() {
        // 终结符的FIRST集是其自身
        for (const Symbol& t : terminals) {
            firstSet[t].insert(t);
        }
        // 空符号E的FIRST集
        Symbol epsilon("E", true);
        firstSet[epsilon].insert(epsilon);

        // 迭代计算非终结符的FIRST集
        bool changed = true;
        while (changed) {
            changed = false;
            for (const Production& prod : productions) {
                set<Symbol>& first = firstSet[prod.left];
                size_t oldSize = first.size();

                // 计算右部的FIRST集
                bool allHaveEpsilon = true;
                for (const Symbol& sym : prod.right) {
                    if (sym.name == "E") {
                        // 空产生式
                        first.insert(Symbol("E", true));
                        break;
                    }

                    // 将sym的FIRST集（除了ε）加入
                    for (const Symbol& f : firstSet[sym]) {
                        if (f.name != "E") {
                            first.insert(f);
                        }
                    }

                    // 如果sym的FIRST集不包含ε，停止
                    if (firstSet[sym].find(Symbol("E", true)) == firstSet[sym].end()) {
                        allHaveEpsilon = false;
                        break;
                    }
                }

                // 如果所有符号都能推导出ε，则加入ε
                if (allHaveEpsilon && !prod.right.empty() && prod.right[0].name != "E") {
                    first.insert(Symbol("E", true));
                }

                if (first.size() != oldSize) {
                    changed = true;
                }
            }
        }
    }

    // 计算符号串的FIRST集
    set<Symbol> getFirstOfString(const vector<Symbol>& symbols, int start) {
        set<Symbol> result;
        if (start >= (int)symbols.size()) {
            result.insert(Symbol("E", true));
            return result;
        }

        bool allHaveEpsilon = true;
        for (int i = start; i < (int)symbols.size(); i++) {
            const Symbol& sym = symbols[i];
            if (sym.name == "E") {
                result.insert(Symbol("E", true));
                break;
            }

            for (const Symbol& f : firstSet[sym]) {
                if (f.name != "E") {
                    result.insert(f);
                }
            }

            if (firstSet[sym].find(Symbol("E", true)) == firstSet[sym].end()) {
                allHaveEpsilon = false;
                break;
            }
        }

        if (allHaveEpsilon) {
            result.insert(Symbol("E", true));
        }

        return result;
    }

    // 计算FOLLOW集
    void computeFollowSets() {
        // 起始符号的FOLLOW集包含$
        followSet[augmentedStart].insert(Symbol("$", true));
        followSet[startSymbol].insert(Symbol("$", true));

        bool changed = true;
        while (changed) {
            changed = false;
            for (const Production& prod : productions) {
                for (int i = 0; i < (int)prod.right.size(); i++) {
                    const Symbol& B = prod.right[i];
                    if (B.isTerminal || B.name == "E") continue;

                    size_t oldSize = followSet[B].size();

                    // 计算β的FIRST集
                    set<Symbol> firstBeta = getFirstOfString(prod.right, i + 1);

                    // 将FIRST(β) - {ε}加入FOLLOW(B)
                    for (const Symbol& f : firstBeta) {
                        if (f.name != "E") {
                            followSet[B].insert(f);
                        }
                    }

                    // 如果β能推导出ε（或者β为空），将FOLLOW(A)加入FOLLOW(B)
                    if (firstBeta.find(Symbol("E", true)) != firstBeta.end()) {
                        for (const Symbol& f : followSet[prod.left]) {
                            followSet[B].insert(f);
                        }
                    }

                    if (followSet[B].size() != oldSize) {
                        changed = true;
                    }
                }
            }
        }
    }

    // 计算项目集的闭包
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

            // 构造βa串，用于计算向前看符号
            vector<Symbol> betaA;
            for (int i = item.dotPos + 1; i < (int)item.prod.right.size(); i++) {
                if (item.prod.right[i].name != "E") {
                    betaA.push_back(item.prod.right[i]);
                }
            }
            betaA.push_back(item.lookahead);

            // 计算FIRST(βa)
            set<Symbol> firstBetaA = getFirstOfString(betaA, 0);

            // 对每个B的产生式
            for (const Production& prod : productions) {
                if (prod.left == B) {
                    for (const Symbol& la : firstBetaA) {
                        if (la.name == "E") continue;  // 跳过ε
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

    // 计算GOTO(I, X)
    LR1State gotoFunc(const LR1State& state, const Symbol& X) {
        set<LR1Item> newItems;

        for (const LR1Item& item : state.items) {
            Symbol afterDot = item.getSymbolAfterDot();
            if (afterDot == X) {
                LR1Item newItem = item;
                newItem.dotPos++;
                newItems.insert(newItem);
            }
        }

        return closure(newItems);
    }

    // 构建LR(1)自动机
    void buildLR1Automaton() {
        // 初始项目：[program' -> ·program, $]
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

        // BFS构建状态
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

    // 构建分析表
    void buildParsingTable() {
        for (const LR1State& state : states) {
            for (const LR1Item& item : state.items) {
                if (item.isComplete()) {
                    // 规约项目
                    if (item.prod.left == augmentedStart) {
                        // 接受动作
                        actionTable[make_pair(state.id, Symbol("$", true))] = -1;
                    } else {
                        // 规约动作：编码为-(产生式编号+2)
                        int action = -(item.prod.id + 2);
                        actionTable[make_pair(state.id, item.lookahead)] = action;
                    }
                } else {
                    Symbol a = item.getSymbolAfterDot();
                    if (a.isTerminal && a.name != "E") {
                        // 移入动作
                        auto gotoIt = gotoTable.find(make_pair(state.id, a));
                        if (gotoIt != gotoTable.end()) {
                            // 移入动作：编码为正数（目标状态号+1）
                            actionTable[make_pair(state.id, a)] = gotoIt->second + 1;
                        }
                    }
                }
            }
        }
    }

    // 词法分析
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

            // 识别词法单元
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
                // 检查是否是关键字
                if (token == "if" || token == "then" || token == "else" ||
                    token == "while") {
                    tokens.push_back(Token(token, line));
                } else if (token == "ID" || token == "NUM") {
                    // 测试输入中的ID和NUM
                    tokens.push_back(Token(token, line));
                } else {
                    // 普通标识符
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

            // 未知字符，跳过
            i++;
        }

        // 添加结束符
        tokens.push_back(Token("$", line));
    }

    // 获取当前词法单元
    Token getCurrentToken() {
        if (currentToken < (int)tokens.size()) {
            return tokens[currentToken];
        }
        return Token("$", tokens.empty() ? 1 : tokens.back().line);
    }

    // 前进到下一个词法单元
    void advance() {
        if (currentToken < (int)tokens.size()) {
            lastTokenLine = tokens[currentToken].line;
            currentToken++;
        }
    }

    // 将句型转换为字符串
    string sententialFormToString(const vector<Symbol>& form) {
        string result;
        for (int i = 0; i < (int)form.size(); i++) {
            if (i > 0) result += " ";
            if (form[i].name != "E") {
                result += form[i].name;
            }
        }
        return result;
    }

    // 语法分析主函数
    void parse() {
        stack<int> stateStack;      // 状态栈
        stack<Symbol> symbolStack;  // 符号栈

        stateStack.push(0);
        currentToken = 0;

        // 记录规约序列（产生式编号）
        vector<int> reductions;

        while (true) {
            int state = stateStack.top();
            Token tok = getCurrentToken();
            Symbol a(tok.value, true);

            auto actionIt = actionTable.find(make_pair(state, a));

            if (actionIt == actionTable.end()) {
                // 语法错误，尝试错误恢复
                handleError(state, tok, stateStack, symbolStack);
                continue;
            }

            int action = actionIt->second;

            if (action > 0) {
                // 移入动作
                int nextState = action - 1;
                symbolStack.push(a);
                stateStack.push(nextState);
                advance();
            } else if (action == -1) {
                // 接受
                break;
            } else {
                // 规约动作
                int prodId = -(action + 2);
                const Production& prod = productions[prodId];

                // 记录规约
                reductions.push_back(prodId);

                // 弹出右部长度个符号（空产生式不弹出）
                int popCount = prod.right.size();
                if (prod.right.size() == 1 && prod.right[0].name == "E") {
                    popCount = 0;
                }

                for (int i = 0; i < popCount; i++) {
                    symbolStack.pop();
                    stateStack.pop();
                }

                // 压入左部非终结符
                symbolStack.push(prod.left);

                // 查找GOTO表获取新状态
                int topState = stateStack.top();
                auto gotoIt = gotoTable.find(make_pair(topState, prod.left));
                if (gotoIt != gotoTable.end()) {
                    stateStack.push(gotoIt->second);
                }
            }
        }

        // 根据规约序列生成最右推导
        generateDerivation(reductions);
    }

    // 错误处理
    void handleError(int state, const Token& tok, stack<int>& stateStack,
                     stack<Symbol>& symbolStack) {
        // 分析期望的符号
        set<string> expected;
        for (auto& entry : actionTable) {
            if (entry.first.first == state && entry.second != 0) {
                expected.insert(entry.first.second.name);
            }
        }

        // 判断错误类型并恢复
        // 对于缺少的符号，使用上一个词法单元的行号
        if (expected.find(";") != expected.end() && tok.value != ";") {
            // 缺少分号 - 使用上一个token的行号
            string errorMsg = "语法错误，第" + to_string(lastTokenLine) + "行，缺少\";\"";
            errors.push_back(errorMsg);

            // 插入分号继续分析
            Token semicolon(";", lastTokenLine);
            tokens.insert(tokens.begin() + currentToken, semicolon);
        } else if (expected.find(")") != expected.end() && tok.value != ")") {
            // 缺少右括号
            string errorMsg = "语法错误，第" + to_string(lastTokenLine) + "行，缺少\")\"";
            errors.push_back(errorMsg);

            Token rparen(")", lastTokenLine);
            tokens.insert(tokens.begin() + currentToken, rparen);
        } else if (expected.find("}") != expected.end() && tok.value != "}") {
            // 缺少右花括号
            string errorMsg = "语法错误，第" + to_string(lastTokenLine) + "行，缺少\"}\"";
            errors.push_back(errorMsg);

            Token rbrace("}", lastTokenLine);
            tokens.insert(tokens.begin() + currentToken, rbrace);
        } else if (expected.find("then") != expected.end() && tok.value != "then") {
            // 缺少then
            string errorMsg = "语法错误，第" + to_string(tok.line) + "行，缺少\"then\"";
            errors.push_back(errorMsg);

            Token thenTok("then", tok.line);
            tokens.insert(tokens.begin() + currentToken, thenTok);
        } else if (expected.find("else") != expected.end() && tok.value != "else") {
            // 缺少else
            string errorMsg = "语法错误，第" + to_string(tok.line) + "行，缺少\"else\"";
            errors.push_back(errorMsg);

            Token elseTok("else", tok.line);
            tokens.insert(tokens.begin() + currentToken, elseTok);
        } else {
            // 其他错误：跳过当前符号
            string errorMsg = "语法错误，第" + to_string(tok.line) + "行，意外的\"" + tok.value + "\"";
            errors.push_back(errorMsg);
            advance();
        }
    }

    // 生成最右推导
    void generateDerivation(const vector<int>& reductions) {
        // 从起始符号开始
        vector<Symbol> form;
        form.push_back(startSymbol);
        derivation.push_back(sententialFormToString(form));

        // 逆序处理规约序列（因为规约是自底向上的）
        for (int i = reductions.size() - 1; i >= 0; i--) {
            int prodId = reductions[i];
            if (prodId == 0) continue;  // 跳过增广产生式

            const Production& prod = productions[prodId];

            // 在当前句型中找到最右边可以展开的非终结符
            int pos = -1;
            for (int j = form.size() - 1; j >= 0; j--) {
                if (form[j] == prod.left) {
                    // 检查这个位置展开后是否能匹配
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

    // 输出结果
    void printResult() {
        // 先输出错误信息
        for (const string& err : errors) {
            cout << err << endl;
        }

        // 输出推导过程
        for (int i = 0; i < (int)derivation.size(); i++) {
            cout << derivation[i];
            if (i < (int)derivation.size() - 1) {
                cout << " => " << endl;
            } else {
                cout << endl;
            }
        }
    }

    // 主分析函数
    void analyze(const string& prog) {
        tokenize(prog);
        parse();
        printResult();
    }
};

void Analysis()
{
    string prog;
    read_prog(prog);
    /* 骚年们 请开始你们的表演 */
    /********* Begin *********/
    LRParser parser;
    parser.analyze(prog);
    /********* End *********/

}
