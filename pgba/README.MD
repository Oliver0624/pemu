pGBA: mGBA using libcross2d as user interface
===========================

### The main goals of this [mgba](https://github.com/mgba-emu/mgba) port are:
- make mGBA support Chinese: interface and game description
- have a more friendly user interface
- using libretro interface
- will add Chinese game descriptions in gamelist.xml (will not be finished very quickly)

### Currently supported devices
Tested:
- Linux (sdl1, sdl2)
- Nintendo Switch (libnx, sdl2)
Untested:
- Sony PS4 (openorbis)
- Sony Ps Vita (vitasdk, vita2d)

### Capabilities
- Support save states
- Support 2x, 3x, fit, fit 4/3 and fullscreen hardware scaling
- Support hardware filtering (shaders)
- Support shaders effects

### Preview images and videos
- support preview images and videos (put files in roms media folder: png files goes in "roms/media/mixrbv2", mp4 files in "roms/media/video"). Please note that the vita port doesn't support video previews.
- you can scrape images and videos from ScreenScraper.fr database using Skraper.

### Installation (switch)
- copy "pgba" directory to "/switch/" directory on sdcard
- copy roms to "/switch/pgba/roms/" directory on sdcard

### Usage (switch)
- Press (+) in the rom list to configure main options and default roms options
- Press (-) in the rom list to configure per rom options
- Press (+) AND (-) when in emulation to bring the options menu

### Thanks
- ScHlAuChi
- devkitPro and libnx people: yellows8, plutoo, profi200, WntrMute ....
- ScHlAuChi again :)
