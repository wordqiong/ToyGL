#version 410 core
layout (vertices = 4) out;
uniform int tessLevel;
void main(){
	if(gl_InvocationID == 0) {
	   gl_TessLevelOuter[0] = tessLevel;
	   gl_TessLevelOuter[1] = tessLevel;
	   gl_TessLevelOuter[2] = tessLevel;
	   gl_TessLevelOuter[3] = tessLevel;
	   gl_TessLevelInner[0] = tessLevel;//ˮƽ����
	   gl_TessLevelInner[1] = tessLevel;//���·���
	}
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}