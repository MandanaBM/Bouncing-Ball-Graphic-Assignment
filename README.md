

# Compile and Run

```
make && ./bouncy
```

## macOs

Modified Angel.h like below to run on latest macOs (`brew install glew`):
```
#ifdef __APPLE__  // include Mac OS X verions of headers
// #  include <OpenGL/OpenGL.h>
#  include <GL/glew.h>
#  include <GLUT/glut.h>
```

