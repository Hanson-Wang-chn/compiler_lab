# 编译原理实践课程项目

本仓库包含"编译原理"实践课程的4个实验项目，从词法分析到语义分析，完整实现了一个C语言子集编译器的前端。所有项目均使用C++11编写，采用面向对象设计。

## 项目结构

```
compiler_lab/
├── lab_1/    # 实验1：词法分析器
├── lab_2/    # 实验2：LL(1)语法分析器
├── lab_3/    # 实验3：LR(1)语法分析器
└── lab_4/    # 实验4：翻译模式（语法制导翻译）
```

每个实验的补充文档：
- [实验一文档](lab_1/supp.md)
- [实验二文档](lab_2/supp.md)
- [实验三文档](lab_3/supp.md)
- [实验四文档](lab_4/supp.md)

## 环境要求

推荐环境（开发时使用的环境）：
- 操作系统：Ubuntu 22.04
- 编译器：g++ (支持C++11)
- 架构：x86

**NOTE: 在一台安装有 clang 编译器的 ARM 芯片 Mac 上测试运行所有代码，暂未发现问题。**

---

# 实验1：C语言词法分析器

## 项目概述

本项目词法分析器实现，使用C++11编写了一个完整的C语言子集词法分析器。项目采用面向对象设计，基于有限自动机（DFA）理论实现。

## 核心功能

### 基础实验

1. **完整的Token识别**：
   - 32个C语言关键字（auto、break、case等）
   - 标识符（字母、数字、下划线组合）
   - 数字常量（整数、浮点数、科学计数法）
   - 运算符（单字符、双字符、三字符，如<、<=、<<=）
   - 界符（括号、分号等）
   - 注释（块注释`/* */`和行注释`//`）
   - 字符串字面量

2. **技术特点**：
   - 基于DFA（有限自动机）设计
   - 面向对象编程（OOP）
   - 最长匹配原则
   - 高效的关键字查找（map数据结构，O(log n)）
   - 完善的错误处理和鲁棒性

3. **输出格式**：
   每个Token一行，格式为`序号: <符号名,编号>`
   ```
   1: <int,17>
   2: <main,81>
   3: <(,44>
   4: <),45>
   5: <{,59>
   ...
   ```

### 补充实验

1. **自动化测试系统**：
   - 批量测试用例执行
   - 自动结果验证
   - 测试报告生成

2. **Token流可视化工具**：
   - 彩色高亮显示
   - Token统计信息
   - 直观展示词法分析结果

3. **专项功能测试**：
   - 数字识别测试（整数、浮点、科学计数法）
   - 注释处理测试（块注释、行注释）
   - 运算符识别测试（最长匹配）
   - 鲁棒性测试（边界情况、错误处理）

4. **性能基准测试**：
   - 关键字查找性能对比
   - 不同数据结构效率分析

## 快速开始

### 编译和运行

#### 基础实验：直接使用LexAnalysis.h

创建测试文件`main.cpp`：

```cpp
#include "LexAnalysis.h"

int main() {
    Analysis();
    return 0;
}
```

编译运行：

```bash
g++ -std=c++11 -o lexer main.cpp
./lexer < input.c
```

或使用管道输入：

```bash
echo "int main() { return 0; }" | ./lexer
```

#### 补充实验1：运行自动化测试

```bash
cd test_automation
chmod +x run_tests.sh
./run_tests.sh
```

测试系统会自动：
1. 编译词法分析器
2. 运行所有测试用例
3. 比较实际输出与期望输出
4. 生成测试报告

#### 补充实验2：使用可视化工具

```bash
cd visualization_tool
g++ -std=c++11 -o visualizer visualizer.cpp
echo "int main() { return 0; }" | ./visualizer
```

可视化工具会彩色显示Token流，并提供统计信息。

#### 补充实验3：数字识别测试

```bash
cd number_recognizer_test
g++ -std=c++11 -o number_test number_test.cpp
./number_test
```

#### 补充实验4：运算符识别测试

```bash
cd operator_test
g++ -std=c++11 -o operator_test operator_test.cpp
./operator_test
```

#### 补充实验5：注释处理测试

```bash
cd comment_test
g++ -std=c++11 -o comment_test comment_test.cpp
./comment_test
```

#### 补充实验6：鲁棒性测试

```bash
cd robustness_test
g++ -std=c++11 -o robustness_test robustness_test.cpp
./robustness_test
```

#### 补充实验7：性能基准测试

```bash
cd performance_benchmark
g++ -std=c++11 -o benchmark benchmark.cpp
./benchmark
```

## 输入输出示例

### 输入示例

```c
int main() {
    int x = 10;
    // 这是行注释
    /* 这是块注释 */
    return 0;
}
```

### 输出示例

```
1: <int,17>
2: <main,81>
3: <(,44>
4: <),45>
5: <{,59>
6: <int,17>
7: <x,81>
8: <=,72>
9: <10,80>
10: <;,53>
11: <// 这是行注释,79>
12: </* 这是块注释 */,79>
13: <return,28>
14: <0,80>
15: <;,53>
16: <},60>
```

---

# 实验2：LL(1)语法分析器

## 项目概述

本项目是LL(1)语法分析器实现，使用C++11编写了一个完整的自顶向下语法分析器。项目采用面向对象设计，实现了FIRST/FOLLOW集自动计算、分析表自动生成、语法树构建和错误恢复等功能。

## 核心功能

### 基础实验

1. **LL(1)语法分析**：
   - 基于预测分析表的自顶向下解析
   - 支持完整的C语言子集文法
   - 自动生成FIRST和FOLLOW集合
   - 自动构建LL(1)分析表

2. **语法树生成**：
   - 层次化的语法树结构
   - 使用Tab缩进表示层级关系
   - 完整展示推导过程

3. **错误处理**：
   - 同步恢复机制（Panic Mode Recovery）
   - 精确的行号定位
   - 自动错误修正并继续解析
   - 支持缺少分号、括号等常见错误

### 补充实验

1. **精确行号计算**：
   - 仅统计非空行，与用户直觉一致
   - 准确定位语法错误位置

2. **自动化测试系统**：
   - 批量测试用例执行
   - 自动结果比对
   - 测试报告生成

3. **彩色终端可视化**：
   - 不同深度节点使用不同颜色
   - 终结符加粗显示
   - 直观展示语法树结构

## 快速开始

### 编译和运行

#### 基础实验：直接使用LLparser.h

创建测试文件`main.cpp`：

```cpp
#include "LLparser.h"

int main() {
    Analysis();
    return 0;
}
```

编译运行：

```bash
g++ -std=c++11 -o parser main.cpp
echo '{ ID = NUM ; }' | ./parser
```

#### 补充实验1：运行自动化测试

```bash
cd test_automation
g++ -std=c++11 -o test_runner main.cpp
./test_runner < testcases/test1.txt
```

#### 补充实验2：彩色可视化

```bash
cd color_visualization
g++ -std=c++11 -o color_parser main.cpp
echo '{ ID = NUM ; }' | ./color_parser
```

## 输入输出示例

### 输入格式

每个符号之间用空格分隔：

```
{
ID = NUM ;
}
```

### 输出格式

语法树使用Tab缩进表示层级：

```
program
	compoundstmt
		{
		stmts
			stmt
				assgstmt
					ID
					=
					arithexpr
						multexpr
							simpleexpr
								NUM
							multexprprime
								E
						arithexprprime
							E
					;
			stmts
				E
		}
```

### 错误处理示例

输入（缺少分号）：

```
{
while ( ID == NUM )
{
ID = NUM
}
}
```

输出：

```
语法错误,第4行,缺少";"
program
	compoundstmt
		{
		stmts
			stmt
				whilestmt
					while
					(
					boolexpr
						...
					)
					stmt
						compoundstmt
							{
							stmts
								stmt
									assgstmt
										ID
										=
										arithexpr
											...
										;
								stmts
									E
							}
			stmts
				E
		}
```

---

# 实验3：LR(1)语法分析器

## 项目概述

本项目是LR(1)语法分析器实现，使用C++11编写了一个完整的自底向上语法分析器。项目采用面向对象设计，实现了LR(1)自动机构建、ACTION/GOTO表自动生成、最右推导输出和错误恢复等功能。

## 核心功能

### 基础实验

1. **LR(1)语法分析**：
   - 基于LR(1)项目集和ACTION/GOTO表的自底向上解析
   - 支持完整的C语言子集文法
   - 精确的向前看符号处理
   - 自动构建LR(1)自动机

2. **最右推导输出**：
   - 从规约序列重建最右推导过程
   - 完整展示推导每一步
   - 格式化输出推导链

3. **错误处理**：
   - 基于符号插入的恐慌模式恢复
   - 精确的行号定位
   - 自动错误修正并继续解析
   - 支持缺少分号、括号等常见错误

### 补充实验

1. **FIRST/FOLLOW集计算工具**：
   - 自动计算并显示所有非终结符的FIRST集
   - 自动计算并显示所有非终结符的FOLLOW集
   - 可视化展示集合内容

2. **ACTION/GOTO表可视化工具**：
   - 显示完整的分析表
   - 统计表的大小和动作类型
   - 帮助理解LR分析过程

3. **自动化测试系统**：
   - 批量测试用例执行
   - 自动结果比对
   - 测试报告生成

## 快速开始

### 编译和运行

#### 基础实验：直接使用LRparser.h

创建测试文件`test_main.cpp`：

```cpp
#include "LRparser.h"

int main() {
    Analysis();
    return 0;
}
```

编译运行：

```bash
g++ -std=c++11 -o test_parser test_main.cpp
echo '{ ID = NUM ; }' | ./test_parser
```

#### 补充实验1：FIRST/FOLLOW集计算工具

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
FIRST(arithexpr) = { ID, NUM, ( }
...

FOLLOW 集
========================================
FOLLOW(program) = { $ }
FOLLOW(stmt) = { ID, else, if, while, {, } }
...
```

#### 补充实验2：ACTION/GOTO表可视化工具

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

#### 补充实验3：自动化测试

```bash
cd test_script
chmod +x run_tests.sh
./run_tests.sh
```

## 输入输出示例

### 输入格式

每个符号之间用空格分隔：

```
{
ID = NUM ;
}
```

### 输出格式

最右推导过程，每步一行，用`=>`分隔：

```
program =>
compoundstmt =>
{ stmts } =>
{ stmt stmts } =>
{ stmt } =>
{ assgstmt } =>
{ ID = arithexpr ; } =>
{ ID = multexpr arithexprprime ; } =>
{ ID = multexpr ; } =>
{ ID = simpleexpr multexprprime ; } =>
{ ID = simpleexpr ; } =>
{ ID = NUM ; }
```

### 错误处理示例

输入（缺少分号）：

```
{
while ( ID == NUM )
{
ID = NUM
}
}
```

输出：

```
语法错误，第4行，缺少";"
program =>
compoundstmt =>
{ stmts } =>
{ stmt stmts } =>
{ stmt } =>
{ whilestmt } =>
{ while ( boolexpr ) stmt } =>
{ while ( boolexpr ) compoundstmt } =>
{ while ( boolexpr ) { stmts } } =>
{ while ( boolexpr ) { stmt stmts } } =>
{ while ( boolexpr ) { stmt } } =>
{ while ( boolexpr ) { assgstmt } } =>
{ while ( boolexpr ) { ID = arithexpr ; } } =>
...
{ while ( ID == NUM ) { ID = NUM ; } }
```

---

# 实验4：翻译模式（Translation Schema）

## 项目概述

本项目是翻译模式的实现，使用C++11编写了一个完整的语法制导翻译器。项目采用面向对象设计，实现了语法分析、语义分析、类型检查和程序执行等功能，能够解释执行包含变量声明、算术表达式和条件语句的简单程序。

## 核心功能

### 基础实验

1. **语法制导翻译**：
   - 递归下降语法分析
   - 语法分析与语义计算同步进行
   - 支持整数和实数两种数据类型
   - 完整的表达式求值

2. **符号表管理**：
   - 变量声明和查找
   - 类型信息存储
   - 变量值的存储和更新
   - 按声明顺序输出结果

3. **类型检查**：
   - 声明时的类型检查
   - 表达式运算时的类型推导
   - 整数与实数的类型转换规则
   - 除零错误检测

4. **程序执行**：
   - 变量声明初始化
   - 赋值语句执行
   - 算术表达式求值
   - if-else条件分支执行

### 补充实验

1. **双类型值存储机制**：
   - 统一的Value结构体
   - 整数和实数的安全存储
   - 类型标志位区分
   - 自动类型转换接口

2. **行号追踪与错误收集**：
   - 词法分析时追踪行号
   - 错误信息批量收集
   - 格式化错误输出
   - 精确的错误定位

3. **分支跳过解析技术**：
   - if-else语句的条件执行
   - 未执行分支的语法解析
   - 避免副作用
   - skip函数族实现

4. **自动化测试脚本**：
   - Shell脚本批量测试
   - 结果自动验证
   - 测试报告生成

## 快速开始

### 编译和运行

#### 基础实验：直接使用TranslationSchema.h

创建测试文件`main.cpp`：

```cpp
#include "TranslationSchema.h"

int main() {
    Analysis();
    return 0;
}
```

编译运行：

```bash
g++ -std=c++11 -o translator main.cpp
./translator << EOF
int a = 1 ; int b = 2 ; real c = 3.0 ;
{
a = a + 1 ;
b = b * a ;
if ( a < b ) then c = c / 2 ; else c = c / 4 ;
}
EOF
```

#### 补充实验：自动化测试

```bash
cd test_scripts
chmod +x run_tests.sh
./run_tests.sh
```

测试脚本会自动：
1. 编译TranslationSchema.h
2. 运行所有测试用例
3. 比较实际输出与期望输出
4. 生成测试报告

## 输入输出示例

### 输入格式

每个符号之间用空格分隔，变量名用小写字母：

```
int a = 1 ; int b = 2 ; real c = 3.0 ;
{
a = a + 1 ;
b = b * a ;
if ( a < b ) then c = c / 2 ; else c = c / 4 ;
}
```

### 输出格式

按声明顺序输出变量名和值，中间用冒号和空格分隔：

```
a: 2
b: 4
c: 1.5
```

### 类型检查示例

输入（类型错误）：

```
int a = 3 ; int b = 5.73 ; real c = 3.0 ;
{
a = a + 1 ;
b = b + a ;
if ( a < b ) then c = c / 0 ; else c = c / 4 ;
}
```

输出：

```
error message:line 1,realnum can not be translated into int type
error message:line 5,division by zero
```

### 复杂表达式示例

输入：

```
int a = 10 ; int b = 25 ; real c = 2.1 ;
{
a = a + 1 ;
c = c * b ;
b = b * a ;
if ( a == b ) then c = c / 3 ; else c = c / 5 ;
}
```

输出：

```
a: 11
b: 275
c: 10.5
```
