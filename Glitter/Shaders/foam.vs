#version 330 core
//顶点着色器输入结构体
//struct appdata
//{
//    vec4 vertex ;//: POSITION;   //POSITION输入语义，Unity将顶点坐标提交于此变量
//    vec4 uv ;//: TEXCOORD0;      //TEXCOORD0，Unity将模型第一套UV提交于此变量
//};

//顶点着色器输出结构体 （反正会给片段做色器使用）
//struct v2f
//{
//    //UNITY_FOG_COORDS(1)
//    vec4 vertex ;//: SV_POSITION;//输出语义
//    vec4 uv1 ;//: TEXCOORD0;
//    vec4 uv2 ;//: TEXCOORD1;
//    vec4 uv3 ;//: TEXCOORD2;
//    vec2 uv4 ;//: TEXCOORD3;
//};

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec2 TexCoord;
out vec2 uv1;
out vec2 uv2;
out vec2 uv3;
out float w;

//外部变量声明
uniform sampler2D   _MainTex;
uniform vec4      _MainTex_ST;
uniform sampler2D   _Mask;
uniform vec4      _Mask_ST;
uniform sampler2D _NoiseTex;
//uniform vec4       _TintColor;
uniform float       _AlphaDelay;
uniform float       _Speed;
uniform float       _Time;//自从程序开始经过的时间
uniform float       _WaveRange;
uniform float       _NoiseRange;
uniform float       _Layer1OffsetX;
uniform float       _Layer2OffsetX;
uniform float       _Layer3OffsetX;
uniform float       _Layer1OffsetY;
uniform float       _Layer2OffsetY;
uniform float       _Layer3OffsetY;



uniform mat4 mvp;

//UV动画函数
vec2 DelayOffsetUV(vec2 uv, float offset, float offset_y)//输入UV 和偏移量（x，y），外部变量控制速度，范围
{
    float pi = 3.1415926536f;
    float sintime = sin(_Time * _Speed * pi + offset * 0.5f * pi);//余弦函数使UV来回移动,
    float u = (sintime + 1) * 0.5f * _WaveRange + (1 - _WaveRange);
    uv.x += u;
    vec4 noiseColor = texture(_NoiseTex, uv);
    uv.x +=_WaveRange*sin(noiseColor.r*_NoiseRange);
    uv.y += offset_y;
    return uv;
    
}

//顶点着色器
void main()
{
    w=aPos.w;
    //w=0.3;
    float x=aPos.x;
    float y=aPos.z;
    vec2 uv=  TexCoord;
    vec4 vertex = vec4(x, 0.0,  y , 1.0);

    gl_Position =mvp * vertex;

    //逐层偏移
    vec2 inuv = uv;//输入的uv

    vec2 uv_tex1 = DelayOffsetUV(inuv, _Layer1OffsetX, _Layer1OffsetY);
    uv1 = uv_tex1 * vec2(_MainTex_ST.x,_MainTex_ST.y) + vec2(_MainTex_ST.z,_MainTex_ST.w);
 
    vec2 uv_tex2 = DelayOffsetUV(inuv, _Layer2OffsetX, _Layer2OffsetY);
    uv2 = uv_tex2 * vec2(_MainTex_ST.x,_MainTex_ST.y) + vec2(_MainTex_ST.z,_MainTex_ST.w);
    
    vec2 uv_tex3 = DelayOffsetUV(inuv, _Layer3OffsetX, _Layer3OffsetY);
    uv3 = uv_tex3 * vec2(_MainTex_ST.x,_MainTex_ST.y) + vec2(_MainTex_ST.z,_MainTex_ST.w);


}
            