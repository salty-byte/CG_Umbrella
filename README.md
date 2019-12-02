CG Umbrella
====

## Overview
このプログラムは大学時代の講義で作成したプログラムです。  
The umbrella program is written by OpenGL.  
Its canopy is used Bezier curve.  
For a given key press:  
* expand the umbrella
* change the angle
* change the position
* animation

## Demo


## Usage
#### Run 

```sh
$ ./umbrella
```

#### Key bindings
* mode

| Key | Description |
|:----|:----|
| t | Translation mode: You can translate the umbrella using 'x', 'X', 'y', 'Y', 'z', 'Z' keys. |
| r | Rotation mode: You can rotate the umbrella using 'x', 'X', 'y', 'Y', 'z', 'Z' keys. |
| m | Switch animation mode: If true, the umbrella opens and closes automatically. |
| b | Blend mode: If true, the umbrella canopy becomes transparent. |
| f | Fog mode: The screen will be difficult to see as if it was foggy. There are three stages. |
| c | Control points mode: If true, show control points and polygons. |

* handle

| Key | Description |
|:----|:----|
| x | Move in the x-axis direction or Rotate about the x-axis. |
| X | The opposite direction of 'x' key. |
| y | Move in the y-axis direction or Rotate about the y-axis. |
| Y | The opposite direction of 'y' key. |
| z | Move in the z-axis direction or Rotate about the z-axis.|
| Z | The opposite direction of 'z' key. |
| s | Increase the umbrella scale. |
| S | Decrease the umbrella scale. |
| o | Increase the angle of the umbrella frame. |
| O | Decrease the angle of the umbrella frame. |
| Esc | Exit program. |

## Build Requirements
* gcc compiler
* [GLUT](https://www.opengl.org/resources/libraries/glut/) - The OpenGL Utility Toolkit

## Build

```sh
$ make umbrella
```
