// ============================================================
// C语言LL(1)语法分析器 - 彩色可视化版本
// 功能：在终端中以彩色显示语法树，不同层级使用不同颜色
// 编译：g++ -std=c++11 -o color_parser main.cpp
// 运行：./color_parser < input.txt
// ============================================================
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

// ============================================================
// ANSI颜色代码定义
// ============================================================
namespace Color {
    // 前景色
    const string RESET = "\033[0m";
    const string BLACK = "\033[30m";
    const string RED = "\033[31m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string BLUE = "\033[34m";
    const string MAGENTA = "\033[35m";
    const string CYAN = "\033[36m";
    const string WHITE = "\033[37m";

    // 亮色版本
    const string BRIGHT_RED = "\033[91m";
    const string BRIGHT_GREEN = "\033[92m";
    const string BRIGHT_YELLOW = "\033[93m";
    const string BRIGHT_BLUE = "\033[94m";
    const string BRIGHT_MAGENTA = "\033[95m";
    const string BRIGHT_CYAN = "\033[96m";

    // 背景色
    const string BG_RED = "\033[41m";
    const string BG_GREEN = "\033[42m";
    const string BG_YELLOW = "\033[43m";
    const string BG_BLUE = "\033[44m";

    // 样式
    const string BOLD = "\033[1m";
    const string UNDERLINE = "\033[4m";

    // 根据深度获取颜色
    string getColorByDepth(int depth) {
        // 定义颜色循环数组
        static const string colors[] = {
            BRIGHT_RED,      // 深度0 - 红色
            BRIGHT_GREEN,    // 深度1 - 绿色
            BRIGHT_YELLOW,   // 深度2 - 黄色
            BRIGHT_BLUE,     // 深度3 - 蓝色
            BRIGHT_MAGENTA,  // 深度4 - 品红
            BRIGHT_CYAN,     // 深度5 - 青色
            RED,             // 深度6 - 暗红
            GREEN,           // 深度7 - 暗绿
            YELLOW,          // 深度8 - 暗黄
            BLUE             // 深度9 - 暗蓝
        };
        return colors[depth % 10];
    }
}

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
// ============================================================
class Token {
public:
    string type;    // token类型
    string value;   // token的实际值
    int line;       // token所在行号

    Token(const string& t = "", const string& v = "", int l = 1)
        : type(t), value(v), line(l) {}
};

// ============================================================
// Lexer类：词法分析器
// ============================================================
class Lexer {
private:
    vector<string> lines;
    int currentLineIdx;
    size_t posInLine;
    int contentLineNumber;

public:
    Lexer(const string& prog) : currentLineIdx(0), posInLine(0), contentLineNumber(0) {
        stringstream ss(prog);
        string line;
        while (getline(ss, line)) {
            lines.push_back(line);
        }
    }

    bool isEmptyLine(const string& line) {
        for (char c : line) {
            if (c != ' ' && c != '\t' && c != '\r') {
                return false;
            }
        }
        return true;
    }

    vector<Token> tokenize() {
        vector<Token> tokens;

        while (currentLineIdx < (int)lines.size()) {
            string& line = lines[currentLineIdx];

            if (isEmptyLine(line)) {
                currentLineIdx++;
                posInLine = 0;
                continue;
            }

            contentLineNumber++;
            posInLine = 0;

            while (posInLine < line.size()) {
                while (posInLine < line.size() &&
                       (line[posInLine] == ' ' || line[posInLine] == '\t')) {
                    posInLine++;
                }

                if (posInLine >= line.size()) break;

                if (posInLine + 1 < line.size()) {
                    string twoChar = line.substr(posInLine, 2);
                    if (twoChar == "<=" || twoChar == ">=" || twoChar == "==") {
                        tokens.push_back(Token(twoChar, twoChar, contentLineNumber));
                        posInLine += 2;
                        continue;
                    }
                }

                char c = line[posInLine];
                if (c == '{' || c == '}' || c == '(' || c == ')' ||
                    c == ';' || c == '=' || c == '<' || c == '>' ||
                    c == '+' || c == '-' || c == '*' || c == '/') {
                    string s(1, c);
                    tokens.push_back(Token(s, s, contentLineNumber));
                    posInLine++;
                    continue;
                }

                if (isalpha(c) || c == '_') {
                    string word;
                    while (posInLine < line.size() &&
                           (isalnum(line[posInLine]) || line[posInLine] == '_')) {
                        word += line[posInLine];
                        posInLine++;
                    }
                    if (word == "if" || word == "then" || word == "else" || word == "while") {
                        tokens.push_back(Token(word, word, contentLineNumber));
                    } else if (word == "ID" || word == "NUM") {
                        tokens.push_back(Token(word, word, contentLineNumber));
                    } else {
                        tokens.push_back(Token("ID", word, contentLineNumber));
                    }
                    continue;
                }

                if (isdigit(c)) {
                    string num;
                    while (posInLine < line.size() && isdigit(line[posInLine])) {
                        num += line[posInLine];
                        posInLine++;
                    }
                    tokens.push_back(Token("NUM", num, contentLineNumber));
                    continue;
                }

                posInLine++;
            }

            currentLineIdx++;
            posInLine = 0;
        }

        tokens.push_back(Token("$", "$", contentLineNumber));
        return tokens;
    }
};

// ============================================================
// Grammar类：文法定义
// ============================================================
class Grammar {
public:
    map<string, vector<vector<string>>> productions;
    map<string, set<string>> firstSets;
    map<string, set<string>> followSets;
    set<string> nonTerminals;
    set<string> terminals;

    Grammar() {
        initGrammar();
        computeFirstSets();
        computeFollowSets();
    }

private:
    void initGrammar() {
        nonTerminals = {"program", "stmt", "compoundstmt", "stmts", "ifstmt",
                        "whilestmt", "assgstmt", "boolexpr", "boolop",
                        "arithexpr", "arithexprprime", "multexpr", "multexprprime",
                        "simpleexpr"};

        terminals = {"{", "}", "if", "(", ")", "then", "else", "while",
                     "ID", "=", ";", "<", ">", "<=", ">=", "==",
                     "+", "-", "*", "/", "NUM", "E", "$"};

        productions["program"] = {{"compoundstmt"}};
        productions["stmt"] = {{"ifstmt"}, {"whilestmt"}, {"assgstmt"}, {"compoundstmt"}};
        productions["compoundstmt"] = {{"{", "stmts", "}"}};
        productions["stmts"] = {{"stmt", "stmts"}, {"E"}};
        productions["ifstmt"] = {{"if", "(", "boolexpr", ")", "then", "stmt", "else", "stmt"}};
        productions["whilestmt"] = {{"while", "(", "boolexpr", ")", "stmt"}};
        productions["assgstmt"] = {{"ID", "=", "arithexpr", ";"}};
        productions["boolexpr"] = {{"arithexpr", "boolop", "arithexpr"}};
        productions["boolop"] = {{"<"}, {">"}, {"<="}, {">="}, {"=="}};
        productions["arithexpr"] = {{"multexpr", "arithexprprime"}};
        productions["arithexprprime"] = {{"+", "multexpr", "arithexprprime"},
                                         {"-", "multexpr", "arithexprprime"},
                                         {"E"}};
        productions["multexpr"] = {{"simpleexpr", "multexprprime"}};
        productions["multexprprime"] = {{"*", "simpleexpr", "multexprprime"},
                                        {"/", "simpleexpr", "multexprprime"},
                                        {"E"}};
        productions["simpleexpr"] = {{"ID"}, {"NUM"}, {"(", "arithexpr", ")"}};
    }

    set<string> computeFirst(const string& symbol) {
        set<string> result;

        if (terminals.count(symbol)) {
            result.insert(symbol);
            return result;
        }

        if (firstSets.count(symbol) && !firstSets[symbol].empty()) {
            return firstSets[symbol];
        }

        for (const auto& prod : productions[symbol]) {
            if (prod.size() == 1 && prod[0] == "E") {
                result.insert("E");
                continue;
            }

            bool allHaveEpsilon = true;
            for (const auto& s : prod) {
                set<string> firstOfS = computeFirst(s);

                for (const auto& f : firstOfS) {
                    if (f != "E") result.insert(f);
                }

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

    void computeFirstSets() {
        for (const auto& t : terminals) {
            firstSets[t].insert(t);
        }

        for (const auto& nt : nonTerminals) {
            firstSets[nt] = computeFirst(nt);
        }
    }

    void computeFollowSets() {
        for (const auto& nt : nonTerminals) {
            followSets[nt] = set<string>();
        }

        followSets["program"].insert("$");

        bool changed = true;
        while (changed) {
            changed = false;

            for (const auto& prodPair : productions) {
                const string& lhs = prodPair.first;

                for (const auto& prod : prodPair.second) {
                    for (size_t i = 0; i < prod.size(); i++) {
                        const string& B = prod[i];

                        if (nonTerminals.find(B) == nonTerminals.end()) continue;

                        set<string> firstOfBeta;
                        bool betaHasEpsilon = true;

                        for (size_t j = i + 1; j < prod.size(); j++) {
                            const string& beta = prod[j];
                            set<string>& firstBeta = firstSets[beta];

                            for (const auto& f : firstBeta) {
                                if (f != "E") firstOfBeta.insert(f);
                            }

                            if (firstBeta.find("E") == firstBeta.end()) {
                                betaHasEpsilon = false;
                                break;
                            }
                        }

                        for (const auto& f : firstOfBeta) {
                            if (followSets[B].find(f) == followSets[B].end()) {
                                followSets[B].insert(f);
                                changed = true;
                            }
                        }

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
// ============================================================
class ParseTable {
public:
    map<string, map<string, int>> table;
    Grammar& grammar;

    ParseTable(Grammar& g) : grammar(g) {
        buildTable();
    }

private:
    void buildTable() {
        for (const auto& prodPair : grammar.productions) {
            const string& A = prodPair.first;
            const vector<vector<string>>& prods = prodPair.second;

            for (size_t i = 0; i < prods.size(); i++) {
                const vector<string>& alpha = prods[i];

                set<string> firstAlpha;
                bool hasEpsilon = true;

                for (const auto& symbol : alpha) {
                    if (symbol == "E") {
                        break;
                    }

                    set<string>& firstSym = grammar.firstSets[symbol];
                    for (const auto& f : firstSym) {
                        if (f != "E") firstAlpha.insert(f);
                    }

                    if (firstSym.find("E") == firstSym.end()) {
                        hasEpsilon = false;
                        break;
                    }
                }

                for (const auto& a : firstAlpha) {
                    table[A][a] = i;
                }

                if (hasEpsilon || (alpha.size() == 1 && alpha[0] == "E")) {
                    for (const auto& b : grammar.followSets[A]) {
                        if (table[A].find(b) == table[A].end()) {
                            table[A][b] = i;
                        }
                    }
                }
            }
        }
    }

public:
    vector<string> getProduction(const string& nonTerminal, const string& terminal) {
        if (table.count(nonTerminal) && table[nonTerminal].count(terminal)) {
            int idx = table[nonTerminal][terminal];
            return grammar.productions[nonTerminal][idx];
        }
        return vector<string>();
    }

    bool hasEntry(const string& nonTerminal, const string& terminal) {
        return table.count(nonTerminal) && table[nonTerminal].count(terminal);
    }
};

// ============================================================
// 语法树节点类（彩色版本）
// ============================================================
class TreeNode {
public:
    string symbol;
    vector<TreeNode*> children;
    bool isTerminal;  // 标记是否为终结符

    TreeNode(const string& s, bool terminal = false) : symbol(s), isTerminal(terminal) {}

    ~TreeNode() {
        for (auto child : children) {
            delete child;
        }
    }

    void addChild(TreeNode* child) {
        children.push_back(child);
    }

    // 彩色输出语法树
    void printColored(int depth = 0) const {
        string color = Color::getColorByDepth(depth);

        // 输出缩进
        for (int i = 0; i < depth; i++) {
            cout << "\t";
        }

        // 根据节点类型选择样式
        if (isTerminal) {
            // 终结符使用粗体
            cout << color << Color::BOLD << symbol << Color::RESET << endl;
        } else {
            // 非终结符正常显示
            cout << color << symbol << Color::RESET << endl;
        }

        // 输出子节点
        for (const auto& child : children) {
            child->printColored(depth + 1);
        }
    }

    // 普通输出（无颜色）
    void print(int depth = 0) const {
        for (int i = 0; i < depth; i++) {
            cout << "\t";
        }
        cout << symbol << endl;

        for (const auto& child : children) {
            child->print(depth + 1);
        }
    }
};

// ============================================================
// LLParser类（彩色版本）
// ============================================================
class LLParser {
private:
    Grammar grammar;
    ParseTable parseTable;
    vector<Token> tokens;
    size_t currentPos;
    vector<string> errors;
    int lastErrorLine;

public:
    LLParser() : parseTable(grammar), currentPos(0), lastErrorLine(-1) {}

    TreeNode* parse(const string& prog) {
        Lexer lexer(prog);
        tokens = lexer.tokenize();
        currentPos = 0;
        errors.clear();
        lastErrorLine = -1;

        TreeNode* root = parseNonTerminal("program");

        // 彩色输出错误信息
        for (const auto& err : errors) {
            cout << Color::RED << Color::BOLD << err << Color::RESET << endl;
        }

        return root;
    }

private:
    Token& currentToken() {
        return tokens[currentPos];
    }

    int getPrevTokenLine() {
        if (currentPos > 0) {
            return tokens[currentPos - 1].line;
        }
        return tokens[currentPos].line;
    }

    void advance() {
        if (currentPos < tokens.size() - 1) {
            currentPos++;
        }
    }

    TreeNode* parseNonTerminal(const string& nonTerminal) {
        TreeNode* node = new TreeNode(nonTerminal, false);

        string lookahead = currentToken().type;

        if (!parseTable.hasEntry(nonTerminal, lookahead)) {
            handleError(nonTerminal, lookahead, node);
            return node;
        }

        vector<string> production = parseTable.getProduction(nonTerminal, lookahead);

        if (production.size() == 1 && production[0] == "E") {
            node->addChild(new TreeNode("E", true));
            return node;
        }

        for (const auto& symbol : production) {
            if (grammar.nonTerminals.count(symbol)) {
                TreeNode* child = parseNonTerminal(symbol);
                node->addChild(child);
            } else {
                if (currentToken().type == symbol) {
                    node->addChild(new TreeNode(symbol, true));
                    advance();
                } else {
                    handleMissingTerminal(symbol, node);
                }
            }
        }

        return node;
    }

    void handleMissingTerminal(const string& expected, TreeNode* parent) {
        int line = getPrevTokenLine();

        if (line != lastErrorLine || errors.empty()) {
            string errorMsg = "语法错误,第" + to_string(line) + "行,缺少\"" + expected + "\"";
            errors.push_back(errorMsg);
            lastErrorLine = line;
        }

        parent->addChild(new TreeNode(expected, true));
    }

    void handleError(const string& nonTerminal, const string& lookahead, TreeNode* node) {
        if (grammar.followSets[nonTerminal].count(lookahead)) {
            node->addChild(new TreeNode("E", true));
            return;
        }

        if (nonTerminal == "stmts" && lookahead == "}") {
            node->addChild(new TreeNode("E", true));
            return;
        }

        bool canBeEmpty = false;
        for (const auto& prod : grammar.productions[nonTerminal]) {
            if (prod.size() == 1 && prod[0] == "E") {
                canBeEmpty = true;
                break;
            }
        }
        if (canBeEmpty) {
            node->addChild(new TreeNode("E", true));
            return;
        }

        advance();

        if (currentPos < tokens.size() - 1) {
            TreeNode* retry = parseNonTerminal(nonTerminal);
            for (auto child : retry->children) {
                node->addChild(child);
            }
            retry->children.clear();
            delete retry;
        }
    }
};

// ============================================================
// 主分析函数
// ============================================================
void Analysis()
{
    string prog;
    read_prog(prog);

    // 打印标题
    cout << Color::CYAN << Color::BOLD;
    cout << "============================================" << endl;
    cout << "    LL(1)语法分析器 - 彩色可视化输出" << endl;
    cout << "============================================" << endl;
    cout << Color::RESET << endl;

    // 创建解析器并解析
    LLParser parser;
    TreeNode* root = parser.parse(prog);

    // 彩色输出语法树
    if (root) {
        root->printColored();
        delete root;
    }

    cout << endl;
    cout << Color::CYAN << "============================================" << endl;
    cout << Color::RESET;
}

int main() {
    Analysis();
    return 0;
}
