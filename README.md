# [Teddy](https://www.cs.toronto.edu/~jacobson/seminar/igarashi-et-al-1999.pdf) - An OpenGL implementation
A sketching interface which converts the 2D freeform strokes created on the screen to a 3D model in OpenGL. This project is inspired by Teddy (Igarashi et al. , 1999) and aims to recreate it in OpenGL.

## Dependencies
1. [OpenGL](https://www.opengl.org/)
2. [ImGui](https://github.com/ocornut/imgui)
3. [Poly2Tri](https://github.com/greenm01/poly2tri)
4. [glm](https://glm.g-truc.net/)
5. [GLFW](https://www.glfw.org/)

Note that Poly2tri and ImGui are already included in the source code, so you would not need to download it separately.

## Local Deployment
1. Clone the git repository.
2. Make sure that you possess OpenGL library installed, you can check it via the command ```glxinfo | grep "OpenGL version"```.
2. Build the source code using the command: ```make```
3. Use the command ```./teddy``` to start sketching.

## Usage
1. Click the left mouse button and drag your mouse to sketch whatever you like. And as soon as you release the button, Delaunay Triangulation would itself happen.
2. Clear the drawn figures using the clear button.
3. Click the right mouse button to apply pruning.
4. Click the middle mouse button to sew triangles and elevate the edges.
5. Click Shift if you wish to go back to Delaunay Triangulation, Right mouse button for pruning and middle mouse button for sewing the triangles and elevating the edges.
