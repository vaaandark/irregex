# irregex

一个基本支持 ERE 语法的正则库，也可以生成 NFA图 和 re-branch-piece-atom 树，还可以用于词法分析。

## 依赖

- `graphviz` 用于画图。

```console
$ sudo apt install graphviz # for Debian or Ubuntu
$ sudo dnf install graphviz # for Fedora or CentOS
$ sudo pacman -S graphviz # for ArchLinux or Manjaro
```

- 一个可以查看矢量图的图片浏览器(或者一个现代浏览器)。

## 构建

```console
$ cd irregex
$ mkdir build
$ make all
```

会在`./build`目录生成三个可执行文件`re2tree`、`re2graph`、`lex`和`irregex`。

## 测试和使用

### irregex.h

`regex_compile`将正则表达式字符串编译为 NFA ，返回一个 NFA 的有向图。
```c
NFAGraph regex_compile(const char *regex);
```

`regex_execute`将字符串在 NFA 图上正则匹配
```c
int regex_execute(NFAGraph *g, const char *str);
```

`regex_free`释放掉 NFA 的图结构。
```c
void regex_free(NFAGraph *g);
```

### irregex

可以使用`irregex`来测试正则表达式的匹配。

```console
$ ./build/irregex 'a*az' aaaaaz
aaaaaz
```

可以使用`./scripts/test.sh`来测试，通过编辑该文件中如下位置添加测试集。

```bash
# run {regex} {str} {expected}
run 'a' 'a' 'a'
run 'abcdefg' 'abcdefg' 'abcdefg'
```

执行`./scripts/test.sh`

```console
$ ./scripts/test.sh
8 SUCCESS
0 FAILURE
100% passed
```

### lex

**感谢 [jyi](https://github.com/jyi2ya) 提供的基于正则的 lex 处理程序`./src/lex.c`**

可以通过对 C 语言文件的词法分析来测试该正则库，测试脚本为`./scripts/test_lex.sh`，它可以对`./src`目录下的所有文件进行词法分析，并输出测试结果。

```console
$ ./scripts/test_lex.sh
11 SUCCESS
0 FAILURE
100% passed
```

如果要对单个 C 语言文件进行词法分析，可以直接使用`./build/lex`。

```console
$ ./scripts/test_lex.sh hello.c
preprocessor command: [#include <stdio.h>]
int: [int]
identifier: [main]
parentheses: [(]
int: [int]
identifier: [argc]
comma: [,]
char: [char]
star: [*]
identifier: [argv]
square brackets: [[]
square brackets: []]
parentheses: [)]
braces: [{]
identifier: [printf]
parentheses: [(]
string literal: ["Hello, World!"]
parentheses: [)]
semicolon: [;]
return: [return]
octal constant: [0]
semicolon: [;]
braces: [}]
```

### RE2tree

可以查看`./build/RE-tree.svg`文件，它包含了正则表达式编译的中间结果。

```console
$ ./scripts/draw.sh {regexp}
$ for p in ./build/*.svg; do xdg-open $p; done
```

![regex-example](./images/RE-tree.svg)

### RE2graph

其中`re`代表`regex`，`br`代表`branch`，`pi`代表`piece`，`at`代表`atom`。连线上的`x-y`代表匹配次数区间。

上述名词的含义可以通过`man 7 regex`查询。

可以查看`./build/NFA-graph.svg`文件，它包含了正则表达式编译出来的 NFA 图示。

![regex-example](./images/NFA-graph.svg)

## 限制

暂不支持锚点、反向引用、断言功能。

由于设计的原因，对一些可以合并的 NFA 顶点（如 ab|ac 合并为 a(b|c)）没有进行优化。

## 参考

[Implementing Regular Expressions](https://swtch.com/~rsc/regexp/)
