// ACTION/GOTO表可视化工具
// 用于展示LR(1)分析表的内容
#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <queue>
using namespace std;

// ==================== 符号类 ====================
class Symbol {
public:
    string name;
    bool isTerminal;

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

    bool operator==(const Production& other) const {
        return left == other.left && right == other.right;
    }

    bool operator<(const Production& other) const {
        if (left != other.left) return left < other.left;
        return right < other.right;
    }
};

// ==================== LR(1)项目类 ====================
class LR1Item {
public:
    Production prod;
    int dotPos;
    Symbol lookahead;

    LR1Item() : dotPos(0) {}
    LR1Item(const Production& p, int d, const Symbol& la)
        : prod(p), dotPos(d), lookahead(la) {}

    Symbol getSymbolAfterDot() const {
        if (dotPos < (int)prod.right.size() &&
            !(prod.right.size() == 1 && prod.right[0].name == "E")) {
            return prod.right[dotPos];
        }
        return Symbol("", true);
    }

    bool isComplete() const {
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
class LR1State {
public:
    set<LR1Item> items;
    int id;

    LR1State() : id(-1) {}

    bool operator==(const LR1State& other) const {
        return items == other.items;
    }

    bool operator<(const LR1State& other) const {
        return items < other.items;
    }
};

// ==================== 表可视化器类 ====================
class TableVisualizer {
private:
    vector<Production> productions;
    set<Symbol> terminals;
    set<Symbol> nonTerminals;
    Symbol startSymbol;
    Symbol augmentedStart;
    map<Symbol, set<Symbol>> firstSet;
    map<Symbol, set<Symbol>> followSet;
    vector<LR1State> states;
    map<pair<int, Symbol>, int> gotoTable;
    map<pair<int, Symbol>, int> actionTable;

public:
    TableVisualizer() {
        initGrammar();
        computeFirstSets();
        computeFollowSets();
        buildLR1Automaton();
        buildParsingTable();
    }

    // 初始化文法
    void initGrammar() {
        vector<string> termNames = {
            "{", "}", "if", "(", ")", "then", "else", "while",
            "ID", "=", ";", "<", ">", "<=", ">=", "==",
            "+", "-", "*", "/", "NUM", "$"
        };
        for (const string& t : termNames) {
            terminals.insert(Symbol(t, true));
        }

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

    void addProduction(const string& left, const vector<string>& right, int id) {
        Symbol leftSym(left, false);
        vector<Symbol> rightSyms;
        for (const string& r : right) {
            bool isTerm = (terminals.find(Symbol(r, true)) != terminals.end()) || r == "E";
            rightSyms.push_back(Symbol(r, isTerm));
        }
        productions.push_back(Production(leftSym, rightSyms, id));
    }

    void computeFirstSets() {
        for (const Symbol& t : terminals) {
            firstSet[t].insert(t);
        }
        Symbol epsilon("E", true);
        firstSet[epsilon].insert(epsilon);

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
                        if (f.name != "E") first.insert(f);
                    }
                    if (firstSet[sym].find(Symbol("E", true)) == firstSet[sym].end()) {
                        allHaveEpsilon = false;
                        break;
                    }
                }
                if (allHaveEpsilon && !prod.right.empty() && prod.right[0].name != "E") {
                    first.insert(Symbol("E", true));
                }
                if (first.size() != oldSize) changed = true;
            }
        }
    }

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
                if (f.name != "E") result.insert(f);
            }
            if (firstSet[sym].find(Symbol("E", true)) == firstSet[sym].end()) {
                allHaveEpsilon = false;
                break;
            }
        }
        if (allHaveEpsilon) result.insert(Symbol("E", true));
        return result;
    }

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
                        if (f.name != "E") followSet[B].insert(f);
                    }
                    if (firstBeta.find(Symbol("E", true)) != firstBeta.end()) {
                        for (const Symbol& f : followSet[prod.left]) {
                            followSet[B].insert(f);
                        }
                    }
                    if (followSet[B].size() != oldSize) changed = true;
                }
            }
        }
    }

    LR1State closure(const set<LR1Item>& items) {
        LR1State state;
        state.items = items;
        queue<LR1Item> workList;
        for (const LR1Item& item : items) workList.push(item);

        while (!workList.empty()) {
            LR1Item item = workList.front();
            workList.pop();
            Symbol B = item.getSymbolAfterDot();
            if (B.name.empty() || B.isTerminal) continue;

            vector<Symbol> betaA;
            for (int i = item.dotPos + 1; i < (int)item.prod.right.size(); i++) {
                if (item.prod.right[i].name != "E") betaA.push_back(item.prod.right[i]);
            }
            betaA.push_back(item.lookahead);
            set<Symbol> firstBetaA = getFirstOfString(betaA, 0);

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

    void buildLR1Automaton() {
        LR1Item startItem(productions[0], 0, Symbol("$", true));
        set<LR1Item> startItems;
        startItems.insert(startItem);
        LR1State startState = closure(startItems);
        startState.id = 0;
        states.push_back(startState);

        set<Symbol> allSymbols;
        for (const Symbol& t : terminals) {
            if (t.name != "$") allSymbols.insert(t);
        }
        for (const Symbol& nt : nonTerminals) {
            allSymbols.insert(nt);
        }

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

    void buildParsingTable() {
        for (const LR1State& state : states) {
            for (const LR1Item& item : state.items) {
                if (item.isComplete()) {
                    if (item.prod.left == augmentedStart) {
                        actionTable[make_pair(state.id, Symbol("$", true))] = -1;
                    } else {
                        int action = -(item.prod.id + 2);
                        actionTable[make_pair(state.id, item.lookahead)] = action;
                    }
                } else {
                    Symbol a = item.getSymbolAfterDot();
                    if (a.isTerminal && a.name != "E") {
                        auto gotoIt = gotoTable.find(make_pair(state.id, a));
                        if (gotoIt != gotoTable.end()) {
                            actionTable[make_pair(state.id, a)] = gotoIt->second + 1;
                        }
                    }
                }
            }
        }
    }

    // 打印ACTION表统计信息
    void printActionTableStats() {
        cout << "========================================" << endl;
        cout << "        ACTION表统计" << endl;
        cout << "========================================" << endl;
        cout << "状态数量: " << states.size() << endl;
        cout << "ACTION表项数量: " << actionTable.size() << endl;

        int shiftCount = 0, reduceCount = 0, acceptCount = 0;
        for (auto& entry : actionTable) {
            if (entry.second > 0) shiftCount++;
            else if (entry.second == -1) acceptCount++;
            else reduceCount++;
        }
        cout << "移入动作数量: " << shiftCount << endl;
        cout << "规约动作数量: " << reduceCount << endl;
        cout << "接受动作数量: " << acceptCount << endl;
        cout << endl;
    }

    // 打印GOTO表统计信息
    void printGotoTableStats() {
        cout << "========================================" << endl;
        cout << "        GOTO表统计" << endl;
        cout << "========================================" << endl;
        cout << "GOTO表项数量: " << gotoTable.size() << endl;

        map<string, int> gotoBySymbol;
        for (auto& entry : gotoTable) {
            gotoBySymbol[entry.first.second.name]++;
        }
        cout << "各符号的GOTO项数量:" << endl;
        for (auto& entry : gotoBySymbol) {
            cout << "  " << entry.first << ": " << entry.second << endl;
        }
        cout << endl;
    }

    // 打印部分ACTION表内容
    void printActionTableSample(int maxRows = 10) {
        cout << "========================================" << endl;
        cout << "      ACTION表部分内容" << endl;
        cout << "========================================" << endl;

        int count = 0;
        for (auto& entry : actionTable) {
            if (count >= maxRows) break;

            int state = entry.first.first;
            string symbol = entry.first.second.name;
            int action = entry.second;

            cout << "ACTION[" << state << ", " << symbol << "] = ";
            if (action > 0) {
                cout << "S" << (action - 1) << " (移入到状态" << (action - 1) << ")";
            } else if (action == -1) {
                cout << "ACC (接受)";
            } else {
                int prodId = -(action + 2);
                cout << "R" << prodId << " (按产生式" << prodId << "规约)";
            }
            cout << endl;
            count++;
        }
        cout << "..." << endl;
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
    cout << "LR(1)语法分析器 - ACTION/GOTO表可视化工具" << endl;
    cout << "===========================================" << endl << endl;

    TableVisualizer viz;
    viz.printProductions();
    viz.printActionTableStats();
    viz.printGotoTableStats();
    viz.printActionTableSample(15);

    return 0;
}
