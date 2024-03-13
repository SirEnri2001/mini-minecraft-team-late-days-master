
uniform mat4 MVP;

in vec4 vs_Pos;

void main() {
    gl_Position = MVP * vs_Pos;
}
