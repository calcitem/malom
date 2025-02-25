﻿Malom，一个九人摩里斯（及其变体）游戏的玩家和解决方案程序。
版权所有 (C) 2007-2023 Gabor E. Gevay, Gabor Danner

查看我们的网页：
http://compalg.inf.elte.hu/~ggevay/mills/index.php

有关许可信息，请查看"许可证 (gpl-3.0).txt"文件。

您可以在我们的网页上链接的论文中阅读关于程序背后的理论。

这是一个Visual Studio 2017的解决方案。如果您有更新的Visual Studio，我们建议在打开解决方案时升级解决方案和项目。该程序在64位Windows上运行。

规则
我们使用的规则与这里描述的相同：
http://www.flyordie.com/games/help/mill/en/contents.html 或在我们的论文中。

项目概述
Malom_megoldas (C++):解决器程序，为单个工作单元（一个或两个子空间）生成解决方案（.sec或.sec2文件），前提是当前工作单元直接依赖的子空间可用。它还可以（基于命令行选项）验证单个子空间，并可以计算各种统计数据。

控制器 (C#):自动为不同的子空间启动解决器，考虑到它们之间的依赖关系。

Malom3 (VB.Net):你可以使用数据库来玩游戏的GUI（并且它还包含一个启发式（alpha-beta）AI）。

Wrappers (C++/CLI):生成一个dll，由控制器和GUI使用，以访问数据库，并跟踪它们之间的依赖关系（在代码中称为sector_graph）。

分析器 (C++):从解决器为单个子空间生成的.analyze文件中汇总数据。（这部分是一团糟，因为我们随机添加了很多功能。如果你想提取一些我们没有计算的统计数据，那么你可能需要给我们发一封电子邮件。）它还从强解的统计数据中计算子空间的值（.secval文件），这对于超强解是必需的。

MalomAPI (VB.Net):生成一个dll，你可以用它从其他程序访问数据库。更多详细信息请查看MalomAPI目录中的另一个Readme.txt。

common.h
你可以在Malom_megoldas/common.h中设置各种宏和常量。例如，DD表示区分平局，即我们是"超级"吗？常量字符串movegen_file也很重要。这个文件包含移动生成的查找表。你应该确保它的目录存在。

在编译和启动之前，请检查以下清单：
[ ] Debug/Release模式编译（Release模式更快，但没有断言，等）
[ ] 变体
[ ] DD/legacy（即超强或强）
[ ] FULL_BOARD_IS_DRAW
[ ] FULL GRAPH（扩展解决方案）
[ ] 控制器模式（如果你正在启动控制器）：在Controller.cs中，你可以设置控制器模式。在大多数情况下，将其保持在“解决”状态是可以的，因为它会自动切换（当有至少一个线程时）到下一个模式，如果在当前模式中没有剩余的工作。
[ ] 数据库是在工作目录中创建（或查找）的。
[ ] 如果你是从Visual Studio启动，那么你应该检查工作目录设置（在项目属性下的调试中），以确保它指向数据库。
[ ] 如果你正在启动或编译控制器，设置Controller.ExeToStart：如果你是从VS启动控制器，那么需要第一个选项；如果你只是在解决器所在的目录中点击exe来启动它，那么需要第二个。
[ ] common.h中由const string movegen_file指定的目录应该存在。
[ ] 解决器需要“buckets”目录存在于工作目录中，控制器需要“lockfiles”目录。
[ ] .secval文件：如果你在DD模式下启动任何东西，那么你需要在当前目录中有这个。它包含了sector的值，可以通过分析器从强解决方案中计算出来（或者可以从我们的网站上下载）。
[ ] 项目：如果你是从VS启动，你应该指定启动项目（在解决方案资源管理器中右键点击它，设置为启动项目）
[ ] 开关：如果你正在启动分析器，那么有多个可能的开关。（还有，如果你直接启动解决器（不通过控制器））
[ ] 如果你在调试模式下从VS启动控制器，那么确保Controller.ExeToStart指定了你想要的exe，并且已经构建。

请注意，哪个数据库需要完美的游戏取决于以下设置：
-变体
-DD/legacy（.sec2/.sec文件）
-FULL_BOARD_IS_DRAW

Malom_megoldas
主函数在Malom_megoldas.cpp中。它检查命令行参数，初始化日志和一些查找表，然后调用解决、验证或分析函数。关于解决方案算法，参见我们的论文（链接在我们的网站上）。

控制器
Mode变量控制我们是在解决问题，验证还是分析。程序在完成一个之后自动在这些操作之间切换。

第一个进度条计算工作单元（或sector，如果我们处于验证模式或分析模式），而第二个计算游戏状态。你可以手动设置解决器实例的数量，或让程序根据可用内存自动做出决定：它每分钟检查一次可用内存，并增加或减少实例数量，如果有太多（超过Inc）或太少（少于dec）的可用内存。最大值字段应设置为你的机器可用的硬件并行性的最大值（在最简单的情况下，是物理CPU核心的数量，但超线程可以使其翻倍）。

在表单的主要部分，你可以看到当前正在运行（和最近完成）的解决器实例的输出（这些也记录在日志文件中）。

如果你想启动控制器，先看一下2中的检查清单。

封装器
这提供了.net封装器，用于解决方案的一些原生C++部分：
- 哈希函数
- 从数据库文件中读取
- sector_graph（子空间之间的依赖关系）
封装器项目包含了来自Malom_megoldas（解决器）项目的许多源文件。然而，有一个叫做WRAPPER的宏，在几个文件中都依赖于许多#ifdefs。所以，共享源文件的许多功能在两个项目中实际上是不同的（例如，查看sector.cpp）。
哈希函数有点棘手（参见我们论文中引用的Ralph Gasser的博士论文）并使用大的查找表。这些对于一次性的所有sector都不适合在内存中，这在玩游戏时是个问题。我们决定将最近访问的8个sector的查找表保留在内存中（这大约是1GB）。这个代码在Wrappers.cpp中。

用户界面
程序的默认模式是完美的游戏，这意味着使用超强数据库它永远不会犯错误（总是达到至少游戏理论的位置值），并且还试图进入对对手来说困难的位置（后者是“超级”部分，你可以从设置菜单中通过复选框标题“忽略数据库中的平局区别信息”来关闭它）。

程序在右下角打印当前位置的评估（你可以从设置中关闭这个功能）。"NTESC"大致意味着，位置被认为是相等的。它实际上的意思是，通过完美的游戏（关于区分的平局），游戏将以平局结束，其中玩家有相等数量的石头（也就是，非暂态的ESC子空间，在我们的论文中的术语）。如果它打印“L”，你就输了（括号中的第二个数字显示你最多可以推迟多少回合）。如果它打印“W”，你就赢了。在其他情况下它打印的东西像 "?59 (std_8_9_0_0)"。这四个数字确定了通过完美游戏将会被平局的子空间：你将在棋盘上有8颗棋子，电脑将有9颗，而你们俩都没有更多的棋子可以放置。第一个数字(-59)是子空间的值。这个值的范围的中心是0，负数表示你处于不利的情况。这个数字不会增加，因为程序正在完美地进行游戏。（如果你犯错误，那么它会减小，否则它会保持不变。）例如，如果评估是3_6_0_0，那么你就非常接近失败。 "NGM:"后面的数字显示当前位置的最佳步数。

如果你从菜单栏激活 "Advisor"，程序还将打印所有可能移动的值，并用黄色标记最佳移动。

在玩家菜单中，你可以设置玩家的类型："Human"表示用户从用户界面控制玩家，"Computer"指的是启发式引擎（这只适用于标准变体），它使用α-β剪枝（也就是，只看几步之后），而 "Perfect"使用数据库。 "Combined"使用两者：启发式玩家在数据库认为同等好的移动之间进行选择。这使得对手更难犯错误，并实现游戏理论的价值。

运行计算
运行计算至少需要4GB RAM来进行强解决方案，而对于超强解决方案则需要8GB。（我们使用了一台16GB和一台20GB的机器，这使得计算更快，因为可以并行运行更多的解决器实例。）（你可以在启动控制器后退出Visual Studio来节省RAM，如果你在Debug菜单中点击Detach All。）

我们使用了一些实用程序来监控计算：Process Explorer，Resource Monitor，Core Temp。