// C语言词法分析器
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

/* 不要修改这个标准输入函数 */
void read_prog(string& prog)
{
	char c;
	while(scanf("%c",&c)!=EOF){
		prog += c;
	}
}

/* 词法单元类，用于表示识别出的Token */
class Token {
public:
	string name;      // 符号名称
	int code;         // 符号编号

	Token(const string& n, int c) : name(n), code(c) {}
};

/* 词法分析器类，基于有限自动机（DFA）设计 */
class LexicalAnalyzer {
private:
	string input;          // 输入的源程序
	int pos;               // 当前读取位置
	int line;              // 当前行号
	vector<Token> tokens;  // 识别出的所有Token
	map<string, int> keywords;  // 关键字表

	/* 初始化关键字表 */
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

	/* 获取当前字符 */
	char peek() {
		if (pos >= input.length()) return '\0';
		return input[pos];
	}

	/* 消耗当前字符并前进 */
	char advance() {
		if (pos >= input.length()) return '\0';
		return input[pos++];
	}

	/* 向前看n个字符 */
	char peekNext(int n = 1) {
		if (pos + n >= input.length()) return '\0';
		return input[pos + n];
	}

	/* 跳过空白字符 */
	void skipWhitespace() {
		while (pos < input.length() && isspace(peek())) {
			if (peek() == '\n') line++;
			advance();
		}
	}

	/* 识别标识符或关键字（自动机状态转换） */
	Token* recognizeIdentifierOrKeyword() {
		string lexeme;
		// 状态0：开始状态，必须是字母或下划线
		if (isalpha(peek()) || peek() == '_') {
			lexeme += advance();
			// 状态1：接受状态，可以继续接受字母、数字或下划线
			while (isalnum(peek()) || peek() == '_') {
				lexeme += advance();
			}
		}

		// 检查是否为关键字
		if (keywords.find(lexeme) != keywords.end()) {
			return new Token(lexeme, keywords[lexeme]);
		}
		// 否则为标识符
		return new Token(lexeme, 81);
	}

	/* 识别数字常量（自动机状态转换） */
	Token* recognizeNumber() {
		string lexeme;
		bool isFloat = false;

		// 状态0：整数部分
		while (isdigit(peek())) {
			lexeme += advance();
		}

		// 状态1：检查小数点
		if (peek() == '.' && isdigit(peekNext())) {
			isFloat = true;
			lexeme += advance();  // 消耗'.'
			// 状态2：小数部分
			while (isdigit(peek())) {
				lexeme += advance();
			}
		}

		// 状态3：检查指数部分（科学计数法）
		if (peek() == 'e' || peek() == 'E') {
			char next = peekNext();
			if (isdigit(next) || ((next == '+' || next == '-') && isdigit(peekNext(2)))) {
				isFloat = true;
				lexeme += advance();  // 消耗'e'或'E'
				if (peek() == '+' || peek() == '-') {
					lexeme += advance();
				}
				while (isdigit(peek())) {
					lexeme += advance();
				}
			}
		}

		// 检查后缀（如L、U、F等）
		while (peek() == 'L' || peek() == 'l' ||
		       peek() == 'U' || peek() == 'u' ||
		       peek() == 'F' || peek() == 'f') {
			lexeme += advance();
		}

		return new Token(lexeme, 80);  // 常数编号为80
	}

	/* 识别运算符或界符（自动机状态转换） */
	Token* recognizeOperatorOrDelimiter() {
		char c = peek();
		char next = peekNext();

		// 多字符运算符识别（最长匹配原则）
		if (c == '-') {
			advance();
			if (peek() == '-') {
				advance();
				return new Token("--", 34);
			} else if (peek() == '=') {
				advance();
				return new Token("-=", 35);
			} else if (peek() == '>') {
				advance();
				return new Token("->", 36);
			}
			return new Token("-", 33);
		}
		else if (c == '!') {
			advance();
			if (peek() == '=') {
				advance();
				return new Token("!=", 38);
			}
			return new Token("!", 37);
		}
		else if (c == '%') {
			advance();
			if (peek() == '=') {
				advance();
				return new Token("%=", 40);
			}
			return new Token("%", 39);
		}
		else if (c == '&') {
			advance();
			if (peek() == '&') {
				advance();
				return new Token("&&", 42);
			} else if (peek() == '=') {
				advance();
				return new Token("&=", 43);
			}
			return new Token("&", 41);
		}
		else if (c == '*') {
			advance();
			if (peek() == '=') {
				advance();
				return new Token("*=", 47);
			}
			return new Token("*", 46);
		}
		else if (c == '/') {
			advance();
			if (peek() == '=') {
				advance();
				return new Token("/=", 51);
			}
			return new Token("/", 50);
		}
		else if (c == '^') {
			advance();
			if (peek() == '=') {
				advance();
				return new Token("^=", 58);
			}
			return new Token("^", 57);
		}
		else if (c == '|') {
			advance();
			if (peek() == '|') {
				advance();
				return new Token("||", 61);
			} else if (peek() == '=') {
				advance();
				return new Token("|=", 62);
			}
			return new Token("|", 60);
		}
		else if (c == '+') {
			advance();
			if (peek() == '+') {
				advance();
				return new Token("++", 66);
			} else if (peek() == '=') {
				advance();
				return new Token("+=", 67);
			}
			return new Token("+", 65);
		}
		else if (c == '<') {
			advance();
			if (peek() == '<') {
				advance();
				if (peek() == '=') {
					advance();
					return new Token("<<=", 70);
				}
				return new Token("<<", 69);
			} else if (peek() == '=') {
				advance();
				return new Token("<=", 71);
			}
			return new Token("<", 68);
		}
		else if (c == '=') {
			advance();
			if (peek() == '=') {
				advance();
				return new Token("==", 73);
			}
			return new Token("=", 72);
		}
		else if (c == '>') {
			advance();
			if (peek() == '>') {
				advance();
				if (peek() == '=') {
					advance();
					return new Token(">>=", 77);
				}
				return new Token(">>", 76);
			} else if (peek() == '=') {
				advance();
				return new Token(">=", 75);
			}
			return new Token(">", 74);
		}
		// 单字符界符和运算符
		else if (c == '(') {
			advance();
			return new Token("(", 44);
		}
		else if (c == ')') {
			advance();
			return new Token(")", 45);
		}
		else if (c == ',') {
			advance();
			return new Token(",", 48);
		}
		else if (c == '.') {
			advance();
			return new Token(".", 49);
		}
		else if (c == ':') {
			advance();
			return new Token(":", 52);
		}
		else if (c == ';') {
			advance();
			return new Token(";", 53);
		}
		else if (c == '?') {
			advance();
			return new Token("?", 54);
		}
		else if (c == '[') {
			advance();
			return new Token("[", 55);
		}
		else if (c == ']') {
			advance();
			return new Token("]", 56);
		}
		else if (c == '{') {
			advance();
			return new Token("{", 59);
		}
		else if (c == '}') {
			advance();
			return new Token("}", 63);
		}
		else if (c == '~') {
			advance();
			return new Token("~", 64);
		}

		return nullptr;
	}

	/* 处理注释（块注释和行注释两种形式），返回是否成功处理 */
	bool handleComment() {
		if (peek() == '/' && peekNext() == '*') {
			// 块注释 /* */
			string comment = "/*";
			advance();  // 消耗'/'
			advance();  // 消耗'*'

			// 读取注释内容直到找到结束符*/
			while (pos < input.length()) {
				if (peek() == '*' && peekNext() == '/') {
					comment += '*';
					advance();  // 消耗'*'
					comment += '/';
					advance();  // 消耗'/'
					break;
				}
				if (peek() == '\n') line++;
				comment += advance();
			}

			tokens.push_back(Token(comment, 79));
			return true;
		}
		else if (peek() == '/' && peekNext() == '/') {
			// 行注释 //
			string comment = "//";
			advance();  // 消耗第一个'/'
			advance();  // 消耗第二个'/'

			// 读取到行尾
			while (pos < input.length() && peek() != '\n') {
				comment += advance();
			}

			tokens.push_back(Token(comment, 79));
			return true;
		}

		return false;
	}

	/* 处理字符串字面量 */
	void handleString() {
		if (peek() == '"') {
			tokens.push_back(Token("\"", 78));  // 开始引号
			advance();  // 消耗开始引号

			string content;
			// 读取字符串内容直到结束引号
			while (pos < input.length() && peek() != '"') {
				if (peek() == '\\' && peekNext() != '\0') {
					// 处理转义字符
					content += advance();  // 添加'\'
					content += advance();  // 添加转义字符
				} else {
					if (peek() == '\n') line++;
					content += advance();
				}
			}

			// 如果字符串内容不为空，将其作为标识符添加
			if (!content.empty()) {
				tokens.push_back(Token(content, 81));
			}

			// 添加结束引号
			if (peek() == '"') {
				tokens.push_back(Token("\"", 78));
				advance();
			}
		}
	}

	/* 主扫描函数 */
	void scan() {
		while (pos < input.length()) {
			skipWhitespace();

			if (pos >= input.length()) break;

			char c = peek();

			// 处理注释
			if (c == '/' && (peekNext() == '*' || peekNext() == '/')) {
				if (handleComment()) {
					continue;
				}
			}

			// 处理字符串
			if (c == '"') {
				handleString();
				continue;
			}

			// 处理标识符或关键字
			if (isalpha(c) || c == '_') {
				Token* t = recognizeIdentifierOrKeyword();
				if (t != nullptr) {
					tokens.push_back(*t);
					delete t;
				}
				continue;
			}

			// 处理数字
			if (isdigit(c)) {
				Token* t = recognizeNumber();
				if (t != nullptr) {
					tokens.push_back(*t);
					delete t;
				}
				continue;
			}

			// 处理运算符和界符
			Token* t = recognizeOperatorOrDelimiter();
			if (t != nullptr) {
				tokens.push_back(*t);
				delete t;
				continue;
			}

			// 未识别字符，跳过（鲁棒性处理）
			advance();
		}
	}

public:
	/* 构造函数 */
	LexicalAnalyzer(const string& source) : input(source), pos(0), line(1) {
		initKeywords();
	}

	/* 执行词法分析 */
	void analyze() {
		scan();
	}

	/* 输出结果 */
	void output() {
		for (size_t i = 0; i < tokens.size(); i++) {
			cout << (i + 1) << ": <" << tokens[i].name << "," << tokens[i].code << ">";
			// 注意：最后一行后面不能有回车
			if (i < tokens.size() - 1) {
				cout << "\n";
			}
		}
	}
};

/* 你可以添加其他函数 */

void Analysis()
{
	string prog;
	read_prog(prog);
	/* 骚年们 请开始你们的表演 */
    /********* Begin *********/

    // 创建词法分析器对象
    LexicalAnalyzer analyzer(prog);

    // 执行词法分析
    analyzer.analyze();

    // 输出分析结果
    analyzer.output();

    /********* End *********/

}
