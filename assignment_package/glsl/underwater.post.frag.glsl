#version 420
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

layout(binding=1) uniform sampler2D u_RenderedTexture;
layout(binding=2) uniform sampler2D u_DepthTexture;
layout(binding=3) uniform sampler2D u_SkyTexture;
layout(binding=5) uniform sampler2D u_miniMap;
layout(binding=6) uniform sampler2D u_miniMapDepth;
uniform int u_Time;
uniform int u_Effect; // 0: no effect; 1: under water; 2: under lava
// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec2 fs_UV;
in vec4 fs_Pos;
out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

const float mapMinX = 0.05,
            mapMinY = 0.8,
            mapMaxX = 0.2,
            mapMaxY = 0.95;


void main()
{
    if ((fs_UV.x > mapMinX) && (fs_UV.x < mapMaxX) && (fs_UV.y > mapMinY) && (fs_UV.y < mapMaxY)) {

            vec2 map_UV = vec2((fs_UV.x - mapMinX) / (mapMaxX - mapMinX),
                                (fs_UV.y - mapMinY) / (mapMaxY - mapMinY));

            vec4 miniMapColor = texture(u_miniMap,map_UV);
            miniMapColor = vec4(normalize(miniMapColor.xyz) * texture(u_miniMapDepth,map_UV).r+0.1f,1.);
            out_Col = miniMapColor;
//            if (length(map_UV - vec2(0.5,0.5)) < 0.025) {
//                out_Col += vec4(1,0,0,0.5);
//            }
            return;
        }

    vec4 diffuseColor = texture(u_RenderedTexture,fs_UV);
    vec4 skyColor = texture(u_SkyTexture, fs_UV);
    //vec4 depth=texture(u_DepthTexture,fs_UV);
    vec4 test = texelFetch(u_DepthTexture, ivec2(fs_UV), 0);
    float alpha=diffuseColor.a;
    // Compute final shaded color
    out_Col = test;
    if(u_Effect==2){
        diffuseColor= texture(u_RenderedTexture,vec2(fs_UV.x,fs_UV.y+0.01f*sin(0.03141592f*u_Time*2+fs_UV.x*5.f)));
        diffuseColor = vec4(diffuseColor.r,0,0,alpha);
    }else if(u_Effect==1){
        diffuseColor= texture(u_RenderedTexture,vec2(fs_UV.x,fs_UV.y+0.01f*sin(0.03141592f*u_Time*2+fs_UV.x*5.f)));
        diffuseColor = vec4(0,0,diffuseColor.b,alpha);
    }else{
        out_Col = diffuseColor;
    }
    out_Col = mix(skyColor, diffuseColor, alpha);
}
