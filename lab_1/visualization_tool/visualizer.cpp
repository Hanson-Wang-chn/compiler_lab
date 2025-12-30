// Token流可视化工具
// 彩色高亮显示词法分析结果

#include "../LexAnalysis.h"
#include <iomanip>

// ANSI颜色代码
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

/* 增强的Token类，带类型分类 */
class VisualToken {
public:
    string name;
    int code;
    string type;  // 类型：关键字、标识符、常数、运算符、界符、注释

    VisualToken(const string& n, int c) : name(n), code(c) {
        classifyType();
    }

    void classifyType() {
        if (code >= 1 && code <= 32) {
            type = "关键字";
        } else if (code >= 33 && code <= 77) {
            type = "运算符";
        } else if (code == 78) {
            type = "界  符";
        } else if (code == 79) {
            type = "注  释";
        } else if (code == 80) {
            type = "常  数";
        } else if (code == 81) {
            type = "标识符";
        } else {
            type = "未知";
        }
    }

    string getColor() const {
        if (type == "关键字") return BLUE;
        if (type == "标识符") return GREEN;
        if (type == "常  数") return YELLOW;
        if (type == "运算符") return MAGENTA;
        if (type == "界  符") return CYAN;
        if (type == "注  释") return RED;
        return WHITE;
    }
};

/* 可视化词法分析器 */
class VisualLexicalAnalyzer {
private:
    string input;
    int pos;
    vector<VisualToken> tokens;
    map<string, int> keywords;

    // 统计信息
    int keywordCount = 0;
    int identifierCount = 0;
    int numberCount = 0;
    int operatorCount = 0;
    int delimiterCount = 0;
    int commentCount = 0;

    void initKeywords() {
        keywords["auto"] = 1;
        keywords["break"] = 2;
        keywords["case"] = 3;
        keywords["char"] = 4;
        keywords["const"] = 5;
        keywords["continue"] = 6;
        keywords["default"] = 7;
        keywords["do"] = 8;
        keywords["double"] = 9;
        keywords["else"] = 10;
        keywords["enum"] = 11;
        keywords["extern"] = 12;
        keywords["float"] = 13;
        keywords["for"] = 14;
        keywords["goto"] = 15;
        keywords["if"] = 16;
        keywords["int"] = 17;
        keywords["long"] = 18;
        keywords["register"] = 19;
        keywords["return"] = 20;
        keywords["short"] = 21;
        keywords["signed"] = 22;
        keywords["sizeof"] = 23;
        keywords["static"] = 24;
        keywords["struct"] = 25;
        keywords["switch"] = 26;
        keywords["typedef"] = 27;
        keywords["union"] = 28;
        keywords["unsigned"] = 29;
        keywords["void"] = 30;
        keywords["volatile"] = 31;
        keywords["while"] = 32;
    }

    char peek() {
        if (pos >= input.length()) return '\0';
        return input[pos];
    }

    char advance() {
        if (pos >= input.length()) return '\0';
        return input[pos++];
    }

    char peekNext(int n = 1) {
        if (pos + n >= input.length()) return '\0';
        return input[pos + n];
    }

    void skipWhitespace() {
        while (pos < input.length() && isspace(peek())) {
            advance();
        }
    }

    VisualToken* recognizeIdentifierOrKeyword() {
        string lexeme;
        if (isalpha(peek()) || peek() == '_') {
            lexeme += advance();
            while (isalnum(peek()) || peek() == '_') {
                lexeme += advance();
            }
        }

        if (keywords.find(lexeme) != keywords.end()) {
            keywordCount++;
            return new VisualToken(lexeme, keywords[lexeme]);
        }
        identifierCount++;
        return new VisualToken(lexeme, 81);
    }

    VisualToken* recognizeNumber() {
        string lexeme;
        while (isdigit(peek())) {
            lexeme += advance();
        }

        if (peek() == '.' && isdigit(peekNext())) {
            lexeme += advance();
            while (isdigit(peek())) {
                lexeme += advance();
            }
        }

        if (peek() == 'e' || peek() == 'E') {
            char next = peekNext();
            if (isdigit(next) || ((next == '+' || next == '-') && isdigit(peekNext(2)))) {
                lexeme += advance();
                if (peek() == '+' || peek() == '-') {
                    lexeme += advance();
                }
                while (isdigit(peek())) {
                    lexeme += advance();
                }
            }
        }

        while (peek() == 'L' || peek() == 'l' ||
               peek() == 'U' || peek() == 'u' ||
               peek() == 'F' || peek() == 'f') {
            lexeme += advance();
        }

        numberCount++;
        return new VisualToken(lexeme, 80);
    }

    VisualToken* recognizeOperatorOrDelimiter() {
        char c = peek();
        bool isOperator = true;

        if (c == '-') {
            advance();
            if (peek() == '-') { advance(); operatorCount++; return new VisualToken("--", 34); }
            else if (peek() == '=') { advance(); operatorCount++; return new VisualToken("-=", 35); }
            else if (peek() == '>') { advance(); operatorCount++; return new VisualToken("->", 36); }
            operatorCount++;
            return new VisualToken("-", 33);
        }
        else if (c == '!') {
            advance();
            if (peek() == '=') { advance(); operatorCount++; return new VisualToken("!=", 38); }
            operatorCount++;
            return new VisualToken("!", 37);
        }
        else if (c == '%') {
            advance();
            if (peek() == '=') { advance(); operatorCount++; return new VisualToken("%=", 40); }
            operatorCount++;
            return new VisualToken("%", 39);
        }
        else if (c == '&') {
            advance();
            if (peek() == '&') { advance(); operatorCount++; return new VisualToken("&&", 42); }
            else if (peek() == '=') { advance(); operatorCount++; return new VisualToken("&=", 43); }
            operatorCount++;
            return new VisualToken("&", 41);
        }
        else if (c == '*') {
            advance();
            if (peek() == '=') { advance(); operatorCount++; return new VisualToken("*=", 47); }
            operatorCount++;
            return new VisualToken("*", 46);
        }
        else if (c == '/') {
            advance();
            if (peek() == '=') { advance(); operatorCount++; return new VisualToken("/=", 51); }
            operatorCount++;
            return new VisualToken("/", 50);
        }
        else if (c == '^') {
            advance();
            if (peek() == '=') { advance(); operatorCount++; return new VisualToken("^=", 58); }
            operatorCount++;
            return new VisualToken("^", 57);
        }
        else if (c == '|') {
            advance();
            if (peek() == '|') { advance(); operatorCount++; return new VisualToken("||", 61); }
            else if (peek() == '=') { advance(); operatorCount++; return new VisualToken("|=", 62); }
            operatorCount++;
            return new VisualToken("|", 60);
        }
        else if (c == '+') {
            advance();
            if (peek() == '+') { advance(); operatorCount++; return new VisualToken("++", 66); }
            else if (peek() == '=') { advance(); operatorCount++; return new VisualToken("+=", 67); }
            operatorCount++;
            return new VisualToken("+", 65);
        }
        else if (c == '<') {
            advance();
            if (peek() == '<') {
                advance();
                if (peek() == '=') { advance(); operatorCount++; return new VisualToken("<<=", 70); }
                operatorCount++;
                return new VisualToken("<<", 69);
            } else if (peek() == '=') { advance(); operatorCount++; return new VisualToken("<=", 71); }
            operatorCount++;
            return new VisualToken("<", 68);
        }
        else if (c == '=') {
            advance();
            if (peek() == '=') { advance(); operatorCount++; return new VisualToken("==", 73); }
            operatorCount++;
            return new VisualToken("=", 72);
        }
        else if (c == '>') {
            advance();
            if (peek() == '>') {
                advance();
                if (peek() == '=') { advance(); operatorCount++; return new VisualToken(">>=", 77); }
                operatorCount++;
                return new VisualToken(">>", 76);
            } else if (peek() == '=') { advance(); operatorCount++; return new VisualToken(">=", 75); }
            operatorCount++;
            return new VisualToken(">", 74);
        }

        // 界符
        isOperator = false;
        if (c == '(') { advance(); delimiterCount++; return new VisualToken("(", 44); }
        else if (c == ')') { advance(); delimiterCount++; return new VisualToken(")", 45); }
        else if (c == ',') { advance(); delimiterCount++; return new VisualToken(",", 48); }
        else if (c == '.') { advance(); delimiterCount++; return new VisualToken(".", 49); }
        else if (c == ':') { advance(); delimiterCount++; return new VisualToken(":", 52); }
        else if (c == ';') { advance(); delimiterCount++; return new VisualToken(";", 53); }
        else if (c == '?') { advance(); delimiterCount++; return new VisualToken("?", 54); }
        else if (c == '[') { advance(); delimiterCount++; return new VisualToken("[", 55); }
        else if (c == ']') { advance(); delimiterCount++; return new VisualToken("]", 56); }
        else if (c == '{') { advance(); delimiterCount++; return new VisualToken("{", 59); }
        else if (c == '}') { advance(); delimiterCount++; return new VisualToken("}", 63); }
        else if (c == '~') { advance(); delimiterCount++; return new VisualToken("~", 64); }

        return nullptr;
    }

    bool handleComment() {
        if (peek() == '/' && peekNext() == '*') {
            string comment = "/*";
            advance(); advance();
            while (pos < input.length()) {
                if (peek() == '*' && peekNext() == '/') {
                    comment += '*';
                    advance();
                    comment += '/';
                    advance();
                    break;
                }
                comment += advance();
            }
            commentCount++;
            tokens.push_back(VisualToken(comment, 79));
            return true;
        }
        else if (peek() == '/' && peekNext() == '/') {
            string comment = "//";
            advance(); advance();
            while (pos < input.length() && peek() != '\n') {
                comment += advance();
            }
            commentCount++;
            tokens.push_back(VisualToken(comment, 79));
            return true;
        }
        return false;
    }

    void handleString() {
        if (peek() == '"') {
            delimiterCount++;
            tokens.push_back(VisualToken("\"", 78));
            advance();

            string content;
            while (pos < input.length() && peek() != '"') {
                if (peek() == '\\' && peekNext() != '\0') {
                    content += advance();
                    content += advance();
                } else {
                    content += advance();
                }
            }

            if (!content.empty()) {
                identifierCount++;
                tokens.push_back(VisualToken(content, 81));
            }

            if (peek() == '"') {
                delimiterCount++;
                tokens.push_back(VisualToken("\"", 78));
                advance();
            }
        }
    }

    void scan() {
        while (pos < input.length()) {
            skipWhitespace();
            if (pos >= input.length()) break;

            char c = peek();

            if (c == '/' && (peekNext() == '*' || peekNext() == '/')) {
                if (handleComment()) continue;
            }

            if (c == '"') {
                handleString();
                continue;
            }

            if (isalpha(c) || c == '_') {
                VisualToken* t = recognizeIdentifierOrKeyword();
                if (t != nullptr) {
                    tokens.push_back(*t);
                    delete t;
                }
                continue;
            }

            if (isdigit(c)) {
                VisualToken* t = recognizeNumber();
                if (t != nullptr) {
                    tokens.push_back(*t);
                    delete t;
                }
                continue;
            }

            VisualToken* t = recognizeOperatorOrDelimiter();
            if (t != nullptr) {
                tokens.push_back(*t);
                delete t;
                continue;
            }

            advance();
        }
    }

public:
    VisualLexicalAnalyzer(const string& source) : input(source), pos(0) {
        initKeywords();
    }

    void analyze() {
        scan();
    }

    void displayColorful() {
        cout << BOLD << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << RESET << endl;
        cout << BOLD << "词法分析可视化结果" << RESET << endl;
        cout << BOLD << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << RESET << endl;
        cout << endl;

        for (size_t i = 0; i < tokens.size(); i++) {
            cout << setw(3) << (i + 1) << "  ";
            cout << tokens[i].getColor() << "[" << tokens[i].type << "] " << RESET;
            cout << setw(15) << left << tokens[i].name;
            cout << " (编号: " << tokens[i].code << ")" << endl;
        }

        cout << endl;
        displayStatistics();
    }

    void displayStatistics() {
        cout << BOLD << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << RESET << endl;
        cout << BOLD << "统计信息" << RESET << endl;
        cout << BOLD << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << RESET << endl;
        cout << "总Token数: " << tokens.size() << endl;
        cout << "关键字: " << keywordCount << endl;
        cout << "标识符: " << identifierCount << endl;
        cout << "常  数: " << numberCount << endl;
        cout << "运算符: " << operatorCount << endl;
        cout << "界  符: " << delimiterCount << endl;
        cout << "注  释: " << commentCount << endl;
        cout << BOLD << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << RESET << endl;
    }
};

int main() {
    string prog;
    char c;
    while (scanf("%c", &c) != EOF) {
        prog += c;
    }

    VisualLexicalAnalyzer analyzer(prog);
    analyzer.analyze();
    analyzer.displayColorful();

    return 0;
}
