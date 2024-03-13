#version 330
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

in vec4 vs_Pos;
in vec2 vs_UV;
in vec4 vs_Col;
out vec2 fs_UV;
out vec4 fs_Pos;
out vec4 fs_Col;

void main()
{
    fs_UV = vs_UV;
    gl_Position = vs_Pos;
    fs_Pos = vs_Pos;
    fs_Col = vs_Col;
}
