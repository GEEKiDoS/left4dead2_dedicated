# A custom srcds.exe for L4D2
#### English
## Features
- Fixes console output and console title codepage errors
- Console output is colored now
- Fixes a issue with ```COM_ParseLine``` that casues some weird Unknown Command

## Planned Features
- Multiple server profiles switches with ```-profile```

## Compiling
In order to compile this project, You will need cmake and Visual Studio 2019.

Before building, run the setuplibs.ps1 to setup the PolyHook2.

Then fire up the Visual Studio soluation and build with Release x86 profile, you are good to go.

### 中文
## 功能
- 修复控制台输出以及标题乱码
- 控制台彩色输出
- 修复由于 ```COM_ParseLine``` 导致的奇怪的 Unknown Command

## 计划中的功能
- 使用 ```-profile`` 参数不同切换服务器配置


## Thanks to | 特别感谢
- Ochii - This project is modfied from his cso2-launcher project
- PolyHook 2
- Valve Software