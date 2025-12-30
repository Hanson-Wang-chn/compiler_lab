# 自动化测试系统

## 功能说明

本测试系统用于自动化测试词法分析器的正确性，包括：
- 批量运行测试用例
- 自动对比输出结果
- 生成测试报告

## 目录结构

```
test_automation/
├── run_tests.sh           # 自动化测试脚本
├── test_runner.cpp        # 测试运行器程序
├── test_cases/            # 测试用例目录
│   ├── basic.c
│   ├── comments.c
│   ├── operators.c
│   └── keywords.c
└── expected_outputs/      # 期望输出目录
    ├── basic.txt
    ├── comments.txt
    ├── operators.txt
    └── keywords.txt
```

## 使用方法

### 1. 赋予脚本执行权限

```bash
cd test_automation
chmod +x run_tests.sh
```

### 2. 运行测试

```bash
./run_tests.sh
```

### 3. 查看测试结果

脚本会自动：
1. 编译测试程序
2. 运行所有测试用例
3. 对比实际输出和期望输出
4. 显示通过/失败情况
5. 计算通过率

## 测试用例说明

| 测试文件 | 测试内容 | 说明 |
|---------|---------|------|
| basic.c | 基本功能 | 测试关键字、标识符、字符串、常数等基本元素 |
| comments.c | 注释处理 | 测试块注释和行注释的识别 |
| operators.c | 运算符 | 测试各类运算符和复合赋值运算符 |
| keywords.c | 关键字 | 测试所有32个C语言关键字 |

## 添加新测试用例

1. 在`test_cases/`目录下创建新的`.c`文件
2. 在`expected_outputs/`目录下创建对应的`.txt`文件
3. 运行测试脚本即可

## 输出格式

测试通过示例：
```
测试用例 1: basic ... ✓ 通过
测试用例 2: comments ... ✓ 通过
```

测试失败示例：
```
测试用例 3: operators ... ✗ 失败
  期望输出: expected_outputs/operators.txt
  实际输出: output.tmp
```

## 技术特点

1. **自动化**：一键运行所有测试
2. **可扩展**：轻松添加新测试用例
3. **详细报告**：显示失败原因和差异
4. **统计信息**：计算通过率
