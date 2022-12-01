
## Getting Started


```bash
git clone https://github.com/wordqiong/ToyGL.git
mkdir build
cd build
```

Now generate a project file or makefile for your platform. If you want to use a particular IDE, make sure it is installed; don't forget to set the Start-Up Project in Visual Studio or the Target in Xcode.

```bash
# UNIX Makefile
cmake ..

# Mac OSX
cmake -G "Xcode" ..

# Microsoft Windows
cmake -G "Visual Studio 17" ..
cmake -G "Visual Studio 17 Win64" ..
...
```
note：cmake -G "Visual Studio 17" .. depends on your visual studio version，I use visual studio 2022 ，so 17 can work

and when you successfully init it，you also need to set glitter as start program （启动项目）


