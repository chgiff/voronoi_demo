#version 330 core 
in vec3 fragNor;
in vec3 fragViewPos;

//point light info
in vec3 pointLightVec;
in vec3 pointLightHalfVec;
uniform vec3 pointLightColor;

//directional light color
uniform vec3 dirLightVec;
in vec3 dirLightHalfVec;
uniform vec3 dirLightColor;

//material info
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;

uniform int drawMode;

//texture
in vec2 vTexCoord;
uniform sampler2D terrainTex;

out vec4 color;

in vec3 viewVec;

uniform int fogMode;
const vec3 fogColor = vec3(0.5, 0.5,0.6);
const float FogDensity = 0.03;



void main()
{
	if(dot(viewVec, fragNor) < 0){
		//discard;
	}

	vec3 mat_amb = MatAmb;
	vec3 mat_dif = MatDif;
	vec3 mat_spec = MatSpec;
	float mat_shine = shine;

	if(drawMode == 2){
		mat_amb = (texture(terrainTex, vTexCoord).rgb/8);
		mat_dif = (texture(terrainTex, vTexCoord).rgb);
		mat_spec = (texture(terrainTex, vTexCoord).rgb/8);
		mat_shine = 0.1;
	}

	//point light contribution
	float diffuse = max(0,dot(normalize(fragNor), pointLightVec));
	float spec = max(0,dot(normalize(fragNor), pointLightHalfVec));
	vec3 pointColor = pointLightColor*(mat_amb + mat_dif*diffuse + mat_spec*pow(spec, mat_shine));

	//directional light contribution
	diffuse = max(0,dot(normalize(fragNor), dirLightVec));
	spec = max(0,dot(normalize(fragNor), dirLightHalfVec));
	vec3 dirColor = dirLightColor*(mat_amb + mat_dif*diffuse + mat_spec*pow(spec, mat_shine));


	vec3 finalColor = pointColor + dirColor;

	if(fogMode != 0){
		//calculate fog
		float dist = length(fragViewPos);
		float fogFactor = 1.0 / exp(dist * FogDensity);
		fogFactor = clamp( fogFactor, 0.1, 1.0 );

		finalColor = mix(fogColor, pointColor + dirColor, fogFactor);
	}

	color = vec4(finalColor, 1.0);

	if (drawMode == 1) {
		color = vec4(1.0);
	}
}
