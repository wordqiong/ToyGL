#version 410 core
out vec4 fragColor;
const int SHINENESS=512;
const vec3 WATER_COLOR=vec3(0.04,0.45,0.92);
struct Light{
	vec3 direction;
	vec3 diffuse;
	vec3 ambient;
	vec3 specular;
};
uniform sampler2D normalMap1;
uniform sampler2D normalMap2;
uniform sampler2D wavesNormalMap;
uniform sampler2D water;
uniform sampler2D reflectMap;
uniform sampler2D depthMap;
uniform sampler2D noise;
uniform Light light;
uniform float interpolateFactor;
uniform float wavesOffset;


uniform mat4 projection;
uniform mat4 view;
in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    //vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    mat3 TBNinverse;
} fs_in;

in vec3 mySkyColor;
const int noiseTextureResolution=128;
uniform int worldTime;
//vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);
vec3 calcLight(Light light,vec3 material,vec3 viewDir,vec3 normal);
vec3 calcNormal(vec2 texCoords);
vec4 getwaves(vec4 color,vec2 texCoords);
float getCaustics();
vec3 rayTrace(vec3 startPoint,vec3 direction);
void main(){
	vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	vec2 texCoords=fs_in.TexCoords;
	//texCoords = ParallaxMapping(texCoords,viewDir);
	vec3 normal = calcNormal(texCoords);
	vec3 material =mix(texture(water,texCoords).rgb,mySkyColor,0.5);
	material=mix(material,WATER_COLOR,0.8);
	vec4 lightColor = vec4(calcLight(light,material,viewDir,normal),1.0);
	//焦散
	float caustics =getCaustics();
	lightColor.rgb*=1.0+caustics*0.25;
	lightColor=getwaves(lightColor,texCoords);
	//lightColor =mix(lightColor,vec4(mySkyColor,1.0),0.6);
	
	//reflect
	//vec4 screen_pos=projection*view*vec4(fs_in.FragPos,1.0);
	//vec2 corr_screen_pos_refraction=screen_pos.xy*0.5/screen_pos.w+vec2(0.5,0.5);
	//vec2 corr_screen_pos_reflection=vec2(corr_screen_pos_refraction.x,1-corr_screen_pos_refraction.y);
	
	//vec4 reflectColor=texture(reflectMap,fs_in.TexCoords);
	//vec4 reflectColor=texture(reflectMap,corr_screen_pos_reflection);
	
	vec3 newnormal=normalize(fs_in.TBNinverse*normal);
	vec3 reflectionDirection = reflect(mat3(view)*fs_in.FragPos,newnormal);
	vec3 reflectColor=rayTrace(mat3(view)*fs_in.FragPos,reflectionDirection);
	
	//透射计算
	float cosine=dot(normalize(fs_in.TangentFragPos-fs_in.TangentViewPos),normal);
	cosine = clamp(abs(cosine),0,1);
	float factor=0.05+(1-0.05)*pow(1.0-cosine,5);
	
	lightColor=mix(lightColor*0.3,lightColor,factor);
	fragColor=vec4(mix(lightColor.rgb,reflectColor,0.2).rgb,0.75);

}
vec3 calcNormal(vec2 texCoords){
	vec3 normal1 = texture(normalMap1,texCoords).rgb;
	vec3 normal2 = texture(normalMap2,texCoords).rgb;
	vec3 waves = texture(wavesNormalMap,texCoords).rgb;
	vec3 normal = mix(normal1,normal2,interpolateFactor).rgb;
	normal = mix(normal,waves,0.3).rgb;
	normal =normal.rgb*2-1.0;
	return normalize(normal);
}
vec3 calcLight(Light light, vec3 material, vec3 viewDir, vec3 normal){
	vec3 fragToLightDir = normalize(-light.direction);
	float diff =max(dot(fragToLightDir, normal),0.0);
	vec3 halfwayDir = normalize(fragToLightDir + viewDir);
	float spec = pow(max(dot(halfwayDir,normal),0.0),SHINENESS);
	vec3 ambient = light.ambient * material;
	vec3 diffuse = light.diffuse * material *diff;
	vec3 specular = light.specular * spec;
	return ambient + diffuse + specular;
}
/*
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 10;
    const float maxLayers = 20;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * depth; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue1 = texture(heightMap1, currentTexCoords).r;
    float currentDepthMapValue2 = texture(heightMap2, currentTexCoords).r;
    float currentDepthMapValue3 = texture(wavesHeightMap, currentTexCoords).r;
    float currentDepthMapValue =  mix(currentDepthMapValue1,currentDepthMapValue2,interpolateFactor);
    currentDepthMapValue =  mix(currentDepthMapValue,currentDepthMapValue3,0.3);
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
	currentDepthMapValue1 = texture(heightMap1, currentTexCoords).r;
        currentDepthMapValue2 = texture(heightMap2, currentTexCoords).r;
	currentDepthMapValue3 = texture(wavesHeightMap, currentTexCoords).r;
        currentDepthMapValue =  mix(currentDepthMapValue1,currentDepthMapValue2,interpolateFactor);
        currentDepthMapValue =  mix(currentDepthMapValue,currentDepthMapValue3,0.3);
	// get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    // -- parallax occlusion mapping interpolation from here on
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepthMapValue1 = texture(heightMap1, prevTexCoords).r;
    float beforeDepthMapValue2 = texture(heightMap2, prevTexCoords).r;
    float beforeDepthMapValue3 = texture(wavesHeightMap, currentTexCoords).r;
    float beforeDepthMapValue =  mix(beforeDepthMapValue1,beforeDepthMapValue2,interpolateFactor);
    beforeDepthMapValue =  mix(beforeDepthMapValue,beforeDepthMapValue3,0.3);
    float beforeDepth = beforeDepthMapValue - currentLayerDepth + layerDepth;
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
    return finalTexCoords;
}
*/
vec4 getwaves(vec4 color,vec2 texCoords){
	float speed1 =float(worldTime)/(noiseTextureResolution*15);
	vec2 coord1=texCoords/noiseTextureResolution;
	coord1.x*=3;
	coord1.x+=speed1;
	coord1.y+=speed1*0.2;
	float noise1=texture(noise,coord1).x;
	float speed2=float(worldTime)/(noiseTextureResolution*7);
	vec2 coord2 =texCoords/noiseTextureResolution;
	coord2.x*=0.5;
	coord2.x-=speed2*0.15+noise1*0.05;
	coord2.y-=speed2*0.7-noise1*0.05;
	float noise2 =texture(noise,coord2).x;
	color*=noise2*0.6+0.4;
	return color;
}
float getCaustics() {
    
    // 波纹1
    
    float speed1 = float(worldTime) / (noiseTextureResolution * 15);

    vec2 coord1 = fs_in.TexCoords/ noiseTextureResolution;

    coord1.x *= 4;

    coord1.x += speed1*2 + coord1.y;

    coord1.y -= speed1;

    float noise1 = texture(noise, coord1).x;

    noise1 = noise1*2 - 1.0;


    // 波纹2

    float speed2 =  float(worldTime) / (noiseTextureResolution * 15);

    vec2 coord2 = fs_in.TexCoords/ noiseTextureResolution;

    coord2.y *= 4;

    coord2.y += speed2*2 + coord2.x;

    coord2.x -= speed2;

    float noise2 = texture(noise, coord2).x;

    noise2 = noise2*2 - 1.0;


    return noise1 + noise2; // 叠加

}
vec3 rayTrace(vec3 startPoint,vec3 direction){
	vec3 point=startPoint;
	int iteration=40;
	for(int i=0;i<iteration;i++){
		point +=direction*0.1;
		vec4 pos_screen=projection*vec4(point,1.0);
		pos_screen.xyz /= pos_screen.w;
		pos_screen.xyz =pos_screen.xyz*0.5+0.5;
		if(pos_screen.x<0||pos_screen.x>1||pos_screen.y<0||pos_screen.y>1){return vec3(0);}
		float depth = texture(depthMap,pos_screen.st).x;
		if(depth<pos_screen.z||i==iteration-1){
			return texture(reflectMap,pos_screen.st).rgb;
		}
	}
	return vec3(0);
}