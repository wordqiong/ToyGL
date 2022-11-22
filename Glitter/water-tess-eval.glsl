#version 410 core
layout (quads, equal_spacing, ccw) in;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 viewPos;
uniform float wavesOffset;
out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    //vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    mat3 TBNinverse;
} vs_out;

const vec3 normal=vec3(0.0,1.0,0.0);
const vec3 tangent=vec3(1.0,0.0,0.0);
const vec3 bitangent=vec3(0.0,0.0,1.0);

uniform int worldTime;
vec3 skyColorArr[24] = {
    
    vec3(0.1, 0.6, 0.9),        // 0-1000

    vec3(0.1, 0.6, 0.9),        // 1000 - 2000

    vec3(0.1, 0.6, 0.9),        // 2000 - 3000

    vec3(0.1, 0.6, 0.9),        // 3000 - 4000

    vec3(0.1, 0.6, 0.9),        // 4000 - 5000
 
   vec3(0.1, 0.6, 0.9),        // 5000 - 6000

    vec3(0.1, 0.6, 0.9),        // 6000 - 7000

    vec3(0.1, 0.6, 0.9),        // 7000 - 8000

    vec3(0.1, 0.6, 0.9),        // 8000 - 9000

    vec3(0.1, 0.6, 0.9),        // 9000 - 10000

    vec3(0.1, 0.6, 0.9),        // 10000 - 11000

    vec3(0.1, 0.6, 0.9),        // 11000 - 12000

    vec3(0.1, 0.6, 0.9),        // 12000 - 13000

    vec3(0.02, 0.2, 0.27),      // 13000 - 14000

    vec3(0.02, 0.2, 0.27),      // 14000 - 15000

    vec3(0.02, 0.2, 0.27),      // 15000 - 16000

    vec3(0.02, 0.2, 0.27),      // 16000 - 17000

    vec3(0.02, 0.2, 0.27),      // 17000 - 18000

    vec3(0.02, 0.2, 0.27),      // 18000 - 19000

    vec3(0.02, 0.2, 0.27),      // 19000 - 20000

    vec3(0.02, 0.2, 0.27),      // 20000 - 21000

    vec3(0.02, 0.2, 0.27),      // 21000 - 22000

    vec3(0.02, 0.2, 0.27),      // 22000 - 23000

    vec3(0.02, 0.2, 0.27)       // 23000 - 24000(0)

};
out vec3 mySkyColor;

uniform sampler2D heightMap1;
uniform sampler2D heightMap2;
uniform sampler2D wavesHeightMap;
uniform float interpolateFactor;
uniform float depth;

float calcHeight(vec2 tc);
void main(){
	vec2 tc=gl_in[0].gl_Position.xz;
	tc=vec2(tc.x+gl_TessCoord.x/64.0,tc.y+gl_TessCoord.y/64.0);
	vec4 tessellatedPoint = vec4(tc.x*2-1, calcHeight(tc), tc.y*2-1, 1.0);
	
	gl_Position = projection*view*model * tessellatedPoint;
	vs_out.FragPos = vec3(model * tessellatedPoint);
	vs_out.TexCoords=vec2(tc.x,tc.y+wavesOffset);

	mat3 normalMatrix = transpose(inverse(mat3(model)));
    	vec3 T = normalize(normalMatrix * tangent);
    	vec3 B = normalize(normalMatrix * bitangent);
    	vec3 N = normalize(normalMatrix * normal);    
    
    	mat3 TBN = transpose(mat3(T, B, N));
	vs_out.TBNinverse=mat3(T,B,N);  
    	//vs_out.TangentLightPos = TBN * lightPos;
    	vs_out.TangentViewPos  = TBN * viewPos;
    	vs_out.TangentFragPos  = TBN * vs_out.FragPos;
	int hour =worldTime/1000;
	int next =(hour+1<24)?(hour+1):0;
	float delta=float(worldTime-hour*1000)/1000;
	mySkyColor=mix(skyColorArr[hour],skyColorArr[next],delta);
}

float calcHeight(vec2 tc){
	float height1 = texture(heightMap1, vec2(tc.x+wavesOffset/8,tc.y+wavesOffset/2)).r;
	float height2 = texture(heightMap2, vec2(tc.x+wavesOffset/4,tc.y+wavesOffset/4)).r;
	float wavesHeight = texture(wavesHeightMap, vec2(tc.x+wavesOffset/2,tc.y+wavesOffset/8)).r;
	float height = mix(height1,height2, interpolateFactor);
	height = mix(height, wavesHeight,0.3);
	height=height*2-1;
	height *= depth;
	return height;
}
