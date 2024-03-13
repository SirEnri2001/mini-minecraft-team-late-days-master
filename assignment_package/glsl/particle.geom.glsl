#version 330 core

uniform mat4 u_Model;
uniform mat4 u_ViewProj;
uniform mat4 u_LookAt;
uniform int u_Weather;
layout (points) in;
layout (line_strip, max_vertices = 12) out;

void main() {
    if(u_Weather==1){
        gl_Position = gl_in[0].gl_Position + vec4(0.0, -0.1, 0.0, 0.0);
        gl_Position  = u_ViewProj* gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + vec4(0.0,0.1, 0.0, 0.0);
        gl_Position  = u_ViewProj* gl_Position;
        EmitVertex();

        EndPrimitive();
    }else if(u_Weather==2){
        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(0.0, -0.01, 0.0, 0.0);
        gl_Position  = u_ViewProj* gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(-0.005*sqrt(3.0), -0.005, 0.0, 0.0);
        gl_Position  = u_ViewProj*gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(-0.005*sqrt(3.0), 0.005, 0.0, 0.0);
        gl_Position  = u_ViewProj*gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(0.0,0.01, 0.0, 0.0);
        gl_Position  = u_ViewProj*gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(0.005*sqrt(3.0),0.005, 0.0, 0.0);
        gl_Position  = u_ViewProj*gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(0.005*sqrt(3.0),-0.005, 0.0, 0.0);
        gl_Position  = u_ViewProj*gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(0.0, -0.01, 0.0, 0.0);
        gl_Position  = u_ViewProj*gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(0.0,0.01, 0.0, 0.0);
        gl_Position  = u_ViewProj* gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(-0.005*sqrt(3.0), 0.005, 0.0, 0.0);
        gl_Position  = u_ViewProj*gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(0.005*sqrt(3.0),-0.005, 0.0, 0.0);
        gl_Position  = u_ViewProj*gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(0.005*sqrt(3.0),0.005, 0.0, 0.0);
        gl_Position  = u_ViewProj*gl_Position;
        EmitVertex();

        gl_Position = gl_in[0].gl_Position + u_LookAt*vec4(-0.005*sqrt(3.0), -0.005, 0.0, 0.0);
        gl_Position  = u_ViewProj*gl_Position;
        EmitVertex();

        EndPrimitive();
    }

}
