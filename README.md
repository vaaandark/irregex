# irregex

一个基本支持 ERE 语法的正则库（尚未完成，但可以生成 NFA图 和 re-branch-piece-atom 树）

## 依赖

- `graphviz` 用于画图

```console
$ sudo apt install graphviz # for Debian or Ubuntu
$ sudo dnf install graphviz # for Fedora or CentOS
$ sudo pacman -S graphviz # for ArchLinux or Manjaro
```

- 一个可以查看矢量图的图片浏览器(或者一个现代浏览器)

## 构建

```console
$ make all
```

会在`./build`目录生成两个可执行文件`re2tree`和`re2graph`

## 使用

```console
$ ./scripts/draw.sh {regexp}
$ for p in ./build/*.svg; do xdg-open $p; done
```

可以查看`./build/RE-tree.svg`文件，它包含了正则表达式编译的中间结果。

![regex-example](./images/RE-tree.svg)

其中`re`代表`regex`，`br`代表`branch`，`pi`代表`piece`，`at`代表`atom`。连线上的`x-y`代表匹配次数区间。

上述名词的含义可以通过`man 7 regex`查询。

可以查看`./build/NFA-graph.svg`文件，它包含了正则表达式编译出来的 NFA 图示。

![regex-example](./images/NFA-graph.svg)

## 缺陷

~~疯狂内存泄漏，暂时懒得管了。~~

~~现在使用了很丑陋的方式解决了内存泄漏问题。~~

~~现在正着手处理内存泄漏问题，还有点漏...~~

较为优雅地解决了内存泄漏问题。

发现没有 ε 边处理时会非常不方便，所以又请回来了 ε 。

## 文档编写中...

