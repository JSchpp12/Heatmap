#version 450 

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal; 
layout(location = 2) in vec3 inColor;		//vertex color
layout(location = 3) in vec2 inTexCoord;	//texture coordinate for vertex 
layout(location = 4) in vec3 inTangent; 
layout(location = 5) in vec3 inBiTangent; 
//per veterx materials
layout(location = 6) in vec3 inMatAmbient; 
layout(location = 7) in vec3 inMatDiffuse; 
layout(location = 8) in vec3 inMatSpecular; 
layout(location = 9) in float inMatShininess; 

layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
	mat4 proj;
	mat4 view;  
	mat4 inverseView; 
	int numLights;
	int renderSettings; 
} globalUbo; 

//TODO: combine with above 
layout(binding = 0, set = 1) uniform uniformBufferObject{
	mat4 modelMatrix; 
	mat4 normalModelMatrix; 
} objectUbo;

layout(binding = 0, set = 2) uniform sampler2D computeTexture; 

layout(location = 0) out vec3 outFragColor; 
layout(location = 1) out vec2 outFragTextureCoordinate; 
layout(location = 2) out vec3 outFragPositionWorld;	//fragment's position in world space
layout(location = 3) out vec3 outFragNormalWorld;		//fragment's normal in world space 
layout(location = 4) out vec3 outFragMatAmbient; 
layout(location = 5) out vec3 outFragMatDiffuse; 
layout(location = 6) out vec3 outFragMatSpecular; 
layout(location = 7) out float outFragMatShininess; 
layout(location = 8) out mat3 outTBNMat; 

const float displaceAmt = 25.0; 

vec3 hsv_to_rgb(in vec3 hsv){
	vec3 rgb_prime = vec3(0.0); 

	//hsv vector assumption 
	//1 - hue (angle)
	//2 - brightness
	//3 - saturation
	float H = max(hsv.x * 255, 0.0); 
	// float H = 120.0; 

	float c = hsv.y * hsv.z; 
	float x = c * (1.0 - abs(mod(H/60.0, 2.0) - 1.0)); 
	float m = hsv.y - c; 

	//dont like if but...
	if (H >= 0 && H < 60){
		rgb_prime = vec3(c, x, 0);
	}else if (H >= 60 && H < 120){
		rgb_prime = vec3(x, c, 0);
	}else if (H >= 120 && H < 180){
		rgb_prime = vec3(0, c, x); 
	}else if (H >= 180 && H < 240){
		rgb_prime = vec3(0, x, c); 
	}else if (H >= 240 && H < 300){
		//will never be larger than 255 because texture constraints 4bits
		rgb_prime = vec3(x, 0, c); 
	}

	// return vec3(x); 
	// return vec3(c); 
	// return vec3(m); 
    return vec3(rgb_prime.x + m, rgb_prime.y + m, rgb_prime.z + m); 
}

void main() {
    vec3 raw_comp = vec3(texture(computeTexture, inTexCoord)); 
    vec3 rgb_color = hsv_to_rgb(raw_comp);
	vec4 positionWorld = objectUbo.modelMatrix * vec4(inPosition, 1.0); 
    positionWorld.y += displaceAmt * (raw_comp.x / 255);
	//calculate TBN mat for use from translating sampled normal matrix from Tangent space to model space
	vec3 T = normalize(vec3(objectUbo.modelMatrix * vec4(inTangent, 0.0)));
	vec3 B = normalize(vec3(objectUbo.modelMatrix * vec4(inBiTangent, 0.0)));
	vec3 N = normalize(vec3(objectUbo.modelMatrix * vec4(inNormal, 0.0))); 
	//Gram-Schmidt process to re-orthogonalize TBN vectors
	T = normalize(T - dot(T, N) * N); 

	gl_Position = globalUbo.proj * globalUbo.view * positionWorld;
	//pass through needed properties to fragments
    outFragNormalWorld = normalize(mat3(objectUbo.normalModelMatrix) * inNormal);
	outFragPositionWorld = positionWorld.xyz; 
	outFragColor = rgb_color; 
	outFragTextureCoordinate = inTexCoord; 
	outFragMatAmbient = inMatAmbient; 
	outFragMatDiffuse = inMatDiffuse; 
	outFragMatSpecular = inMatSpecular; 
	outFragMatShininess = inMatShininess; 
    outTBNMat = mat3(T, B, N); 
}