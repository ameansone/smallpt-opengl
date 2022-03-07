# smallpt-opengl

99 lines of code implement monte-carlo path tracing in opengl

![screenshot](https://github.com/ameansone/smallpt-opengl/raw/master/screenshot.png)

> 有点漏光

opengl port of [smallpt](https://www.kevinbeason.com/smallpt).

60 lines of glsl, 39 lines of c++, 80 columns or less

using [glad](https://glad.dav1d.de) and require [glfw library](https://www.glfw.org/)

usage:
```sh
mkdir build
cd build
cmake ../
make
./smallpt
```
