## pEMU模拟器汉化版

**背景**

PEMU系列模拟器（pnes/psnes/pgen/pfbneo）是使用自制的GUI库调用著名开源模拟器内核的模拟器前端。在Switch平台上，是retroarch（全能模拟器）的一个替代选择，其使用的开源模拟器内核的兼容性好于任天堂官方模拟器。
但是PEMU系列模拟器只支持英语，准确地说，它不支持多字节编码（包括GBK/UTF-8）的文字和字体。作者似乎没有考虑支持其他语言，在代码中直接将显示的文本写死了，而且文本和配置项是耦合的。
本次汉化工作针对PEMU系列模拟器进行了改进，使其支持UTF-8编码的文本，进而显示中文游戏列表和简介。在此基础上，做了一些改进，以增强易用性。

**功能说明**
- 基于 6.7 版，后续会跟进官方的修改
- 汉化程序界面
- 支持 gamelist.xml 使用中文（必须为UTF-8编码）
- 游戏列表支持按汉语拼音排序
- 游戏列表支持长名称滚动显示
- 游戏介绍文本框支持翻页（使用LB和RB键）
- 进入/退出游戏时支持自动读档/存档
- pnes 支持对磁碟机游戏进行“换面”操作
- pnes/psnes/pgen 支持光枪
- psnes 支持鼠标（但不完善）
- pgen 打开对 megadrive 以外机种的 rom 的支持
- 添加 pgba ，使用 0.10.5 mgba 内核

**发布说明**
- 全系列发布
- 这个前端使用文件名进行匹配，因此家用机游戏暂时仅支持老男人网站的rom文件名
- 暂未整理汉化游戏列表

**关于内建的gamelist**
- 家用机的游戏名是我自己查资料整理校对的，街机版的游戏名取自 fbneo 整合包
- 红白机的部分内容取自《红白机完全档案》，街机的部分内容取自天马G前端。
- Megadrive 的资料还在持续整理中

**下载地址**
- [Switch汉化版](https://drive.google.com/drive/folders/1j-C3CLtyPjLatxhQVJqAGC6ECaEmHDUA?usp=sharing)
- [封面图](https://drive.google.com/drive/folders/1ziw5VpA-U17kTeetDAz2-lyaWudYM-OS?usp=sharing)
