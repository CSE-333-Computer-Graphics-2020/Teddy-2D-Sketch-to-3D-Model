# Teddy - An OpenGL implementation of [Igarashi's 1999 paper](https://www.cs.toronto.edu/~jacobson/seminar/igarashi-et-al-1999.pdf)
A sketching interface which converts the 2D freeform strokes created on the screen to a 3D model in OpenGL.

## Dependencies
1. [OpenGL](https://www.opengl.org/)
2. [ImGui](https://github.com/ocornut/imgui)
3. [Poly2Tri](https://github.com/greenm01/poly2tri)
Note that Poly2tri and ImGui are already included in the source code, so you would not need to download it separately.

## Local Deployment
1. Clone the git repository.
2. Make sure that you possess OpenGL library installed, you can check it via the command ```glxinfo | grep "OpenGL version"```.
2. Build the source code using the command: ```make```
3. Use the command ```./teddy``` to start sketching.

## Usage
1. Click the left mouse button and drag your mouse to sketch whatever you like.
