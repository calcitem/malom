使用 MalomAPI.dll

要使用 MalomAPI.dll，数据库文件和 Wrappers.dll 应位于当前工作目录中。当构建 Malom（而不是使用下载的二进制文件）时，你可以在 x64 目录下找到 Wrappers.dll。

MalomAPI.dll 是一个 64 位的 DLL。

MalomAPI.dll 提供了 MalomSolutionAccess 类。无需实例化该类，因为所有方法都是静态的。

在以下描述中，“白色” 表示先手玩家，“黑色” 表示后手玩家。

# 函数

这是一个 Nine Men's Morris 游戏中，获取最优 move 的函数：

```c++
int GetBestMove(int whiteBitboard, int blackBitboard, int whiteStonesToPlace,
                int blackStonesToPlace, int playerToMove, bool onlyStoneTaking);
```

此函数接收当前游戏状态，并随机返回最优的一步。

## 参数

- whiteBitboard：棋盘上的白色棋子，以位棋盘编码：低 24 位中的每一位对应棋盘上的一个位置。关于位之间的映射，请参见 Bitboard.png。例如，整数 131 意味着棋盘左侧有一个垂直的三连，因为 131 = 1 + 2 + 128。
- blackBitboard：棋盘上的黑色棋子。
- whiteStonesToPlace：白色玩家还可以在棋盘上放置的棋子数量。
- blackStonesToPlace：黑色玩家还可以在棋盘上放置的棋子数量。
- playerToMove：如果轮到白色玩家行棋，则为 0；如果轮到黑色玩家行棋，则为 1。
- onlyStoneTaking：如果你希望将闭合三连(mill)和移除棋子作为一个单一的动作，那么总是将此设置为 false。如果你将其设置为 true，那么假定刚刚形成了一个三连，只返回要移除的棋子。

## 返回值

函数的返回值是一个32位的位棋盘（bitboard），但实际值用到低24位。这个bitboard表示棋盘上的变化。在 Nine Men's Morris 游戏中，每一步可能会增加一个棋子（放置），也可能会移动一个棋子（滑动或跳跃），还可能会消除一个棋子（吃子）。这个返回值就是用来描述这些变化的。

位棋盘中的每一位都对应棋盘上的一个位置，如果这一位被设置（值为1），那就表示这个位置上的棋子发生了变化：

1. 如果这个位置原来是空的，那么在这一步之后，这个位置上会出现一个棋子。这个棋子是当前玩家的，表示玩家在这一步放置了一个棋子。
2. 如果这个位置原来有一个棋子，那么在这一步之后，这个棋子会消失。如果这个棋子是对手的，那就表示这一步是吃子。如果这个棋子是当前玩家的，那就表示这一步是当前玩家从把棋子这个位置滑动或跳跃走开。

总的来说，这个函数的返回值是一个描述棋盘变化的位棋盘，每一位的变化表示相应的位置上的棋子变化。如果这个位被设置为1，那么要么是这个位置原来是空的，现在放上了一个棋子；要么是这个位置原来有一个棋子，现在这个棋子消失了。

## 转换

现在需要将返回值 move 的格式进行转换，调整为，

返回值 move  是32位的int，以当前行棋方为视角。

如果是吃子，则 move 是负数，move 的绝对值就是被吃掉的子的位置。

如果是放置棋子，move 是正数，move 的值就是棋子放置的位置。

如果是移动棋子，move 是正数，其低 0-7 位存储棋子的新位置，其低 8-15 位存储棋子旧位置。

转换函数的原型：

std::vector<int> convertBitboardMove(int whiteBitboard, int blackBitboard, int playerToMove, int moveBitboard);

如果返回值是两个move连在一起，这种情况下是先是形成mill(可以通过移动棋子或者摆子形成mill)，然后是吃子。这种情况下，应该是返回一个int数组，也就是说，返回值这个vector有2个元素。

一般情况下，返回值vector只有一个元素。

实现的时候，应该全部遍历完所有位，搜集完信息后才能判断这个move是属于吃子还是走子，是否是形成mill后吃子，然后根据情况再来转换。

## 异常

如果参数非法，或者游戏已经结束，此函数会抛出异常。

GetBestMoveNoException 是一个类似的函数，如果你在处理.NET 异常（例如，由于从非.NET 语言调用）时遇到困难，它可能会有用。它的参数与 GetBestMove 相同，但它永远不会抛出异常。它的返回值几乎与 GetBestMove 的返回值相同，但如果有错误，它会返回 0。在这种情况下，你可以调用 GetLastError，它将返回错误作为.NET 字符串。

（我们的函数都不是线程安全的。这意味着你的程序不应该从不同的线程调用我们的函数。）

我们的库可能会消耗大约 1-2 GB 的内存。
