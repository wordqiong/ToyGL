#version 330 core

uniform sampler2D   _MainTex;
uniform vec4      _MainTex_ST;
uniform sampler2D   _Mask;
uniform vec4      _Mask_ST;
uniform vec4       _TintColor;
uniform float       _AlphaDelay;
uniform float       _Speed;
uniform float       _Time;//自从程序开始经过的时间
uniform float       _WaveRange;
uniform float       _Layer1OffsetX;
uniform float       _Layer2OffsetX;
uniform float       _Layer3OffsetX;
uniform float       _Layer1OffsetY;
uniform float       _Layer2OffsetY;
uniform float       _Layer3OffsetY;

uniform vec2 rand_num;

in vec4 uv1;
in vec4 uv2;
in vec4 uv3;
in vec2 uv4;

out vec4 fragColor;

//获取泡沫逐渐出现，向岸边移动，开始折返并逐渐消失的透明度值
float GetDisappearAlpha(float delay)
{
    float PI = 3.1415926536f;
    float t = _Time *_Speed * PI + delay * 0.5* PI + 1.2 * PI;
    float a = (sin(t)+1)*0.5;
    return a*a; 
}

//将两层半透明的颜色合并，获取合并后的RGBA
vec4 TwoColorBlend(vec4 c1, vec4 c2)
{
    vec4 c12;
    float temp_w = c1.w + c2.w - c1.w * c2.w;
    vec3 temp_xyz=(vec3(c1.x,c1.y,c1.z) * c1.w * (1 - c2.w) + vec3(c2.x,c2.y,c2.z) * c2.w) / temp_w;
    //c12.rgb = (c1.rgb * c1.a * (1 - c2.a) + c2.rgb * c2.a) / c12.a;
    c12=vec4(temp_xyz.x,temp_xyz.y,temp_xyz.z,temp_w);
    return c12;
}

//片段着色器
void main() //: SV_Target //参数为输入结构体，语义就是输出到什么地方
{
    float pi = 3.1415926536f;
    
    //get rgb
    vec4 c1 = texture(_MainTex, vec2(uv1.x,uv1.y));
    vec4 c2 = texture(_MainTex, vec2(uv1.z,uv1.w));
    vec4 c3 = texture(_MainTex, vec2(uv2.x,uv2.y));
    c1.w=0.3;
    c2.w=0.3;
    c3.w=0.3;
    //vec4 n1=texture(_Mask, GetDisappearAlpha(_Layer1OffsetX)*vec2(uv2.z,uv2.w) );
    //vec4 n2=texture(_Mask, GetDisappearAlpha(_Layer2OffsetX)*vec2(uv3.x,uv3.y) );
    //vec4 n3=texture(_Mask, GetDisappearAlpha(_Layer3OffsetX)*vec2(uv3.z,uv3.w) );
    //c1=c1+n1;
    //c2=c2+n2;
    //c3=c3+n3;

    //get alpha
    //c1 = texture(_Mask, GetDisappearAlpha(_Layer1OffsetX)*vec2(uv2.z,uv2.w) );
    //c2 = texture(_Mask, GetDisappearAlpha(_Layer2OffsetX)*vec2(uv3.x,uv3.y));
    //c3 = texture(_Mask, GetDisappearAlpha(_Layer3OffsetX)*vec2(uv3.z,uv3.w) );

    //layer1 + layer2
    vec4 c12 = TwoColorBlend(c1,c2);
    //layer12 + layer3
    vec4 c123 = TwoColorBlend(c12,c3);

    fragColor=c123 * _TintColor;
}