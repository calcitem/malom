使用 MalomAPI.dll

要使用 MalomAPI.dll，数据库文件和 Wrappers.dll 应位于当前工作目录中。当构建 Malom（而不是使用下载的二进制文件）时，你可以在 x64 目录下找到 Wrappers.dll。

MalomAPI.dll 是一个 64 位的 DLL。

MalomAPI.dll 提供了 MalomSolutionAccess 类。无需实例化该类，因为所有方法都是静态的。

在以下描述中，“白色” 表示先手玩家，“黑色” 表示后手玩家。

函数：

GetBestMove (whiteBitboard As Integer, blackBitboard As Integer, whiteStonesToPlace As Integer, blackStonesToPlace As Integer, playerToMove As Integer, onlyStoneTaking As Boolean) As Integer

此函数接收当前游戏状态，并随机返回最优的一步。

参数：

- whiteBitboard：棋盘上的白色棋子，以位棋盘编码：前 24 位中的每一位对应棋盘上的一个位置。关于位之间的映射，请参见 Bitboard.png。例如，整数 131 意味着棋盘左侧有一个垂直的三连，因为 131 = 1 + 2 + 128。
- blackBitboard：棋盘上的黑色棋子。
- whiteStonesToPlace：白色玩家还可以在棋盘上放置的棋子数量。
- blackStonesToPlace：黑色玩家还可以在棋盘上放置的棋子数量。
- playerToMove：如果白色玩家要行棋，则为 0；如果黑色玩家要移动，则为 1。
- onlyStoneTaking：如果你希望将闭合三连和移除棋子作为一个单一的动作，那么总是将此设置为 false。如果你将其设置为 true，那么假定刚刚形成了一个三连，只返回要移除的棋子。

返回值： 移动作为位棋盘返回，对于棋盘上的每个变化，都设置了一个位：

- 如果对应设置位的位置为空，则移动玩家的一颗棋子出现在那里。
- 如果对应设置位的位置当前有一颗棋子，那么那颗棋子就消失了。（如果是对手的棋子，那么这个动作涉及到移除棋子。如果是移动玩家的棋子，那么这是一个滑动或跳跃动作，那颗棋子被滑动或跳跃到一个不同的位置。） 如果这增加了移动玩家的棋子数量，那么该玩家在移动后将有一颗少的棋子可以放置。

如果参数非法，或者游戏已经结束，此函数会抛出异常。

GetBestMoveNoException 是一个类似的函数，如果你在处理.NET 异常（例如，由于从非.NET 语言调用）时遇到困难，它可能会有用。它的参数与 GetBestMove 相同，但它永远不会抛出异常。它的返回值几乎与 GetBestMove 的返回值相同，但如果有错误，它会返回 0。在这种情况下，你可以调用 GetLastError，它将返回错误作为.NET 字符串。

（我们的函数都不是线程安全的。这意味着你的程序不应该从不同的线程调用我们的函数。）

我们的库可能会消耗大约 1-2 GB 的内存。
