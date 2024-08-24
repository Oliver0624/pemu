## pEMU模拟器汉化版

**背景**

PEMU系列模拟器（pNES/pSNES/pGEN/pFBNeo）是使用自制的GUI库调用著名开源模拟器内核的模拟器前端。在Switch平台上，是retroarch（全能模拟器）的一个替代选择，其使用的开源模拟器内核的兼容性好于任天堂官方模拟器。
但是PEMU系列模拟器只支持英语，准确地说，它不支持多字节编码（包括GBK/UTF-8）的文字和字体。作者似乎没有考虑支持其他语言，在代码中直接将显示的文本写死了，而且文本和配置项是耦合的。
本次汉化工作针对PEMU系列模拟器进行了改进，使其支持UTF-8编码的文本，进而显示中文游戏列表和简介。在此基础上，做了一些改进，以增强易用性。

**功能说明**
- 中文界面显示（废话），支持gamelist.xml使用中文（必须为UTF-8编码）。修复启动前2秒字体显示模糊的问题。
- 游戏列表的汉字支持按汉语拼音排序
- 游戏介绍文本框支持翻页（使用LB和RB键）
- pNES支持对磁碟机游戏进行“换面”操作，很多磁碟机游戏终于可以玩了（其实nestopia内核支持换面，但是pNES前端缺菜单）
- pGEN模拟器打开对MD以外机种ROM的扫描
- 增加GBA前端（pGBA）：将mGBA内核移植到这个图形前端。支持GB/GBC/GBA 游戏。

**发布说明**
- 发布pNES、pSNES、pFBNeo和pGBA，pGEN汉化版将在不久后发布
- 这个前端使用文件名进行匹配，因此家用机游戏暂时仅支持老男人网站的rom文件名
- 暂未整理汉化游戏列表

**关于内建的gamelist**
- 家用机的游戏名是我自己查资料整理校对的，街机版的游戏名取自fbneo整合包
- 红白机游戏的介绍：一部分是我自己查英日文Wikipedia自己写的，一部分取自《红白机完全档案》，一部分为机翻自screenscraper.fr的文本，有些未整理好。
- GBA游戏的介绍：有600多个游戏简介爬取自英文Wikipedia，并使用AI概括，其余700个仍为英文（因为AI的免费额度用完了...）
- 部分街机游戏的中文简介取自天马G前端。
- 超任的游戏介绍未翻译

**已知问题**
- 游戏列表不支持对长游戏名进行滚动显示，如果游戏名太长，会被截断

**下载地址**
- [Switch汉化版](https://drive.google.com/drive/folders/1j-C3CLtyPjLatxhQVJqAGC6ECaEmHDUA?usp=sharing)
- [封面图](https://drive.google.com/drive/folders/1ziw5VpA-U17kTeetDAz2-lyaWudYM-OS?usp=sharing)
