#version 450 

layout(location = 0) in vec3 inFragColor;
layout(location = 1) in vec2 inFragTextureCoordinate; 
layout(location = 2) in vec3 inFragPositionWorld;
layout(location = 3) in vec3 inFragNormalWorld;
layout(location = 4) in vec3 inFragMatAmbient; 
layout(location = 5) in vec3 inFragMatDiffuse; 
layout(location = 6) in vec3 inFragMatSpecular; 
layout(location = 7) in float inFragMatShininess;
layout(location = 8) in mat3 inTBNMat;

layout(location = 0) out vec4 outColor;

struct RenderSettings{
	uint draw;				//standard draw
	uint drawMatAmbient;  
	uint drawMatDiffuse; 
	uint drawMatSpecular; 
	uint drawMapBump; 

	//rendering features
	uint bumpMapping; 
};

struct Light_Type{
	uint point; 
	uint directional; 
	uint spot;
};

struct Light{
	vec4 position;
	vec4 direction;

	//properties
	vec4 ambient; 
	vec4 diffuse;
	vec4 specular; 
	//controls.x = inner cutoff angle 
	//controls.y = outer cutoff angle
	vec4 controls; 
	//settings.x = enabled
	//settings.y = type
	uvec4 settings; 
};

layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
	mat4 proj;
	mat4 view;  
	mat4 inverseView; 
	uint numLights; 
	uint renderSettings;
} globalUbo; 

 layout(binding = 1, set = 0) buffer globalLightBuffer{
	Light lights[];
 };

layout(binding = 0, set = 2) uniform sampler2D textureSampler; 

//TODO: move struct init to a different place - not good idea to do this for every fragment
RenderSettings createSettingsStruct(){
	RenderSettings settingsChecker = {
		0x0, 
		0x1,
		0x2,
		0x3,
		0x4, 
		//render features
		0x10000
	};
	return(settingsChecker); 
}

//TODO: same move here 
Light_Type createLightTypeStruct(){
	Light_Type lightChecker = {
		0x0, 
		0x1, 
		0x2
	};
	return(lightChecker);
}

vec3 hsv_to_rgb(in vec3 hsv){
	//hsv vector assumption 
	//1 - hue (angle)
	//2 - brightness
	//3 - saturation
	float c = hsv.y * hsv.y; 
	float x = c * (1 - abs(mod(hsv.x/60.0, 2) - 1)); 
	float m = hsv.y - c; 

	//dont like if but...
	if (hsv.x > 0 && hsv.x < 60){
		return vec3(c, x, 0);
	}else if (hsv.x >= 60 && hsv.x < 120){
		return vec3(x, c, 0);
	}else if (hsv.x >= 120 && hsv.x < 180){
		return vec3(0, c, x); 
	}else if (hsv.x >= 180 && hsv.x < 240){
		return vec3(0, x, c); 
	}else{
		//will never be larger than 255 because texture constraints 4bits
		return vec3(x, 0, c); 
	}
}

void main() {
    outColor = vec4(vec3(texture(textureSampler, inFragTextureCoordinate)), 1.0);
}
