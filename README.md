# irregex

一个基本支持 ERE 语法的正则库，也可以生成 NFA图 和 re-branch-piece-atom 树

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

会在`./build`目录生成三个可执行文件`re2tree`、`re2graph`和`irregex`。

## 测试和使用

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

可以查看`./build/RE-tree.svg`文件，它包含了正则表达式编译的中间结果。

```console
$ ./scripts/draw.sh {regexp}
$ for p in ./build/*.svg; do xdg-open $p; done
```

![regex-example](./images/RE-tree.svg)

其中`re`代表`regex`，`br`代表`branch`，`pi`代表`piece`，`at`代表`atom`。连线上的`x-y`代表匹配次数区间。

上述名词的含义可以通过`man 7 regex`查询。

可以查看`./build/NFA-graph.svg`文件，它包含了正则表达式编译出来的 NFA 图示。

![regex-example](./images/NFA-graph.svg)

## 限制

暂不支持锚点、反向引用、断言功能。

## 参考

[Implementing Regular Expressions](https://swtch.com/~rsc/regexp/)
