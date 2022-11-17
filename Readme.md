
## Getting Started


```bash
git clone https://github.com/wordqiong/OpenGL_shader.git
cd Glitter
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


## perform
你可以看到点光源，聚光灯，还有3D模型的展示和天空盒纹理贴图
![image](readme_graph/point_light.png)
![image](readme_graph/SpotLight.png)
![image](readme_graph/3D.png)