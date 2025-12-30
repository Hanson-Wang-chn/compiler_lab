// FIRST集和FOLLOW集计算与输出工具
// 用于展示和验证文法的FIRST集和FOLLOW集
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <queue>
using namespace std;

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
class Production {
public:
    Symbol left;
    vector<Symbol> right;
    int id;

    Production() : id(-1) {}
    Production(const Symbol& l, const vector<Symbol>& r, int i)
        : left(l), right(r), id(i) {}
};

// ==================== FIRST/FOLLOW集计算器类 ====================
class FirstFollowCalculator {
private:
    vector<Production> productions;
    set<Symbol> terminals;
    set<Symbol> nonTerminals;
    Symbol startSymbol;
    Symbol augmentedStart;
    map<Symbol, set<Symbol>> firstSet;
    map<Symbol, set<Symbol>> followSet;

public:
    FirstFollowCalculator() {
        initGrammar();
        computeFirstSets();
        computeFollowSets();
    }

    // 初始化文法（与LRparser.h中相同）
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

        startSymbol = Symbol("program", false);
        augmentedStart = Symbol("program'", false);

        int prodId = 0;
        addProduction("program'", {"program"}, prodId++);
        addProduction("program", {"compoundstmt"}, prodId++);
        addProduction("stmt", {"ifstmt"}, prodId++);
        addProduction("stmt", {"whilestmt"}, prodId++);
        addProduction("stmt", {"assgstmt"}, prodId++);
        addProduction("stmt", {"compoundstmt"}, prodId++);
        addProduction("compoundstmt", {"{", "stmts", "}"}, prodId++);
        addProduction("stmts", {"stmt", "stmts"}, prodId++);
        addProduction("stmts", {"E"}, prodId++);
        addProduction("ifstmt", {"if", "(", "boolexpr", ")", "then", "stmt", "else", "stmt"}, prodId++);
        addProduction("whilestmt", {"while", "(", "boolexpr", ")", "stmt"}, prodId++);
        addProduction("assgstmt", {"ID", "=", "arithexpr", ";"}, prodId++);
        addProduction("boolexpr", {"arithexpr", "boolop", "arithexpr"}, prodId++);
        addProduction("boolop", {"<"}, prodId++);
        addProduction("boolop", {">"}, prodId++);
        addProduction("boolop", {"<="}, prodId++);
        addProduction("boolop", {">="}, prodId++);
        addProduction("boolop", {"=="}, prodId++);
        addProduction("arithexpr", {"multexpr", "arithexprprime"}, prodId++);
        addProduction("arithexprprime", {"+", "multexpr", "arithexprprime"}, prodId++);
        addProduction("arithexprprime", {"-", "multexpr", "arithexprprime"}, prodId++);
        addProduction("arithexprprime", {"E"}, prodId++);
        addProduction("multexpr", {"simpleexpr", "multexprprime"}, prodId++);
        addProduction("multexprprime", {"*", "simpleexpr", "multexprprime"}, prodId++);
        addProduction("multexprprime", {"/", "simpleexpr", "multexprprime"}, prodId++);
        addProduction("multexprprime", {"E"}, prodId++);
        addProduction("simpleexpr", {"ID"}, prodId++);
        addProduction("simpleexpr", {"NUM"}, prodId++);
        addProduction("simpleexpr", {"(", "arithexpr", ")"}, prodId++);
    }

    // 添加产生式
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
                    set<Symbol> firstBeta = getFirstOfString(prod.right, i + 1);

                    for (const Symbol& f : firstBeta) {
                        if (f.name != "E") {
                            followSet[B].insert(f);
                        }
                    }

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

    // 打印FIRST集
    void printFirstSets() {
        cout << "========================================" << endl;
        cout << "           FIRST 集" << endl;
        cout << "========================================" << endl;

        for (const Symbol& nt : nonTerminals) {
            cout << "FIRST(" << nt.name << ") = { ";
            bool first = true;
            for (const Symbol& s : firstSet[nt]) {
                if (!first) cout << ", ";
                cout << s.name;
                first = false;
            }
            cout << " }" << endl;
        }
        cout << endl;
    }

    // 打印FOLLOW集
    void printFollowSets() {
        cout << "========================================" << endl;
        cout << "          FOLLOW 集" << endl;
        cout << "========================================" << endl;

        for (const Symbol& nt : nonTerminals) {
            cout << "FOLLOW(" << nt.name << ") = { ";
            bool first = true;
            for (const Symbol& s : followSet[nt]) {
                if (!first) cout << ", ";
                cout << s.name;
                first = false;
            }
            cout << " }" << endl;
        }
        cout << endl;
    }

    // 打印产生式
    void printProductions() {
        cout << "========================================" << endl;
        cout << "            产生式" << endl;
        cout << "========================================" << endl;

        for (const Production& prod : productions) {
            cout << prod.id << ". " << prod.left.name << " -> ";
            for (size_t i = 0; i < prod.right.size(); i++) {
                if (i > 0) cout << " ";
                cout << prod.right[i].name;
            }
            cout << endl;
        }
        cout << endl;
    }
};

int main() {
    cout << "LR(1)语法分析器 - FIRST/FOLLOW集计算工具" << endl;
    cout << "==========================================" << endl << endl;

    FirstFollowCalculator calc;
    calc.printProductions();
    calc.printFirstSets();
    calc.printFollowSets();

    return 0;
}
