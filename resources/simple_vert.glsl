#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 S;

uniform vec3 eyePos;

uniform vec3 pointLightPos;
uniform vec3 dirLightVec;

out vec3 fragNor;
out vec3 fragViewPos;

out vec3 pointLightVec;
out vec3 pointLightHalfVec;
out vec3 dirLightHalfVec;

out vec3 viewVec;

out vec2 vTexCoord;


void main()
{
	gl_Position = P * V * M * S * vertPos;
	vTexCoord = vec2(vertTex.x, vertTex.y);	

	fragNor = vec3((transpose(inverse(M * S))) * vec4(vertNor, 0.0));
	fragViewPos = vec3(V * M * vertPos);

	//calcuate light vector for point light
	pointLightVec = normalize(pointLightPos - (M * S *vertPos).xyz);

	//calculate specular half vector for point light
	viewVec = normalize(eyePos - (M * S * vertPos).xyz);
	pointLightHalfVec = normalize(pointLightVec + viewVec);

	//calculate specular half vector for directional light
	dirLightHalfVec = normalize(normalize(dirLightVec) + viewVec);
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}
