#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

in vec4 fs_Pos;

out vec4 out_Val;

void main()
{
    // Copy the color; there is no shading.
    out_Val = vec4(vec3(1-fs_Pos[2]/100),1.f);
}
