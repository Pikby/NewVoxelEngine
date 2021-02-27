
#version 450

out vec4 FragColor;
in vec3 Norm;
in vec4 Color;
in vec3 WorldPos;
in vec4 DirectionalShadowPos;


struct DirectionalLight{
	vec3 direction;
	vec3 ambient;
	vec3 specular;
	vec3 diffuse;

	float specularExponent;
	sampler2D shadowTexture;
};

struct PointLight{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	vec3 specularExponent;

	float farPlane;
	samplerCube shadowTexture;
};

uniform DirectionalLight directionalLight;
uniform PointLight[10] pointLight;

uniform float time;
uniform vec3 cameraPos;
uniform int translucent;
uniform int pointLightCount;
uniform sampler2D waterTexture;



float bias = 0.005;

float chebyshevUpperBound(vec4 shadowPos)
{
	vec2 moments = texture2D(directionalLight.shadowTexture,shadowPos.xy).rg;
	
	// Surface is fully lit. as the current fragment is before the light occluder
	if (shadowPos.z <= moments.x)
		return 1.0;

	// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
	// How likely this pixel is to be lit (p_max)
	float variance = moments.y - (moments.x*moments.x);
	//variance = max(variance, 0.000002);
	variance = max(variance, 0.0002);

	float d = shadowPos.z - moments.x;
	float p_max = variance / (variance + d*d);

	return p_max;
}


float shadowCalc(vec4 fragPosShadow){
	vec3 projCoords = fragPosShadow.xyz;
	projCoords = projCoords*0.5 +0.5;
	if(projCoords.z > 1.0) return 0;
	float currentDepth = projCoords.z; 
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(directionalLight.shadowTexture, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(directionalLight.shadowTexture, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;
	return shadow;
}


float calculatePointLightShadow(PointLight light) {
    vec3 fragToLight = WorldPos -light.position; 
    float closestDepth = texture(light.shadowTexture, fragToLight).r;
	
	
	closestDepth *= light.farPlane;  
	float currentDepth = length(fragToLight);

	float bias = 0.05; 
	float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0; 


	return shadow;
}  

vec3 calculatePointLightColor(PointLight light){
    vec3 lightDir = normalize(light.position - WorldPos);
    // diffuse shading
    float diff = max(dot(Norm, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, Norm);
	vec3 viewDir = normalize(cameraPos-WorldPos);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
    float dist    = length(light.position - WorldPos);
    float attenuation = 1.0 / (1 + 0.22 * dist + 0.2 * (dist * dist));    
    // combine results
    vec3 ambient  = light.ambient * attenuation;
    vec3 diffuse  = light.diffuse  * attenuation;
    vec3 specular = light.specular * spec *attenuation;

	float shadow =1-calculatePointLightShadow(light);
    return (shadow)*(ambient +diffuse + specular);
}

vec3 calculateDirectionalLightColor(){
	vec3 diffuse = max(dot(Norm, directionalLight.direction),0.0)*directionalLight.diffuse;
	vec3 viewDir = normalize(cameraPos-WorldPos);
	vec3 reflectDir = reflect(-directionalLight.direction,Norm);
	float spec = pow(max(dot(viewDir,reflectDir),0),directionalLight.specularExponent);
	vec3 specular = 0.5*spec*directionalLight.specular;

	
	vec4 scPostW = DirectionalShadowPos/DirectionalShadowPos.w; 
	scPostW = scPostW * 0.5 + 0.5;
	

	float shadow = 0.0; 
	
	/*
	bool outsideShadowMap = DirectionalShadowPos.w <= 0.0f || (scPostW.x < 0 || scPostW.y < 0) || (scPostW.x >= 1 || scPostW.y >= 1);
	if (!outsideShadowMap) 
	{
		shadow = 1-chebyshevUpperBound(scPostW);
	}
	*/

	//shadow = shadowCalcPoint(WorldPos);
	
	return (directionalLight.ambient+(diffuse+specular));
}

/*
vec3 calculatePointColor(){
	
	
	/*
	bool outsideShadowMap = DirectionalShadowPos.w <= 0.0f || (scPostW.x < 0 || scPostW.y < 0) || (scPostW.x >= 1 || scPostW.y >= 1);
	if (!outsideShadowMap) 
	{
		shadow = 1-chebyshevUpperBound(scPostW);
	}
	

	shadow = shadowCalcPoint(WorldPos);
	
	return (directionalLight.ambientColor+(1-shadow)*(diffuse+specular));
}
*/

void main() {
	vec3 globalColor = calculateDirectionalLightColor();

	vec3 pointColor = vec3(0);
	pointColor += calculatePointLightColor(pointLight[0]);
	pointColor +=  calculatePointLightColor(pointLight[1]);
	//vec3 pointColor = calculatePointLightColor(pointLight[0]);

	FragColor = vec4((pointColor+globalColor)*Color.rgb,Color.a);
	if(translucent == 1){
		vec2 textureOrigin = cameraPos.xz*4-vec2(512);
		vec2 textureCoord = ( WorldPos.xz - textureOrigin);
			FragColor = FragColor*(max((texture(waterTexture,((WorldPos.xz +vec2(time*10))/1024.0f)).r+0.5),1));
	}
	
	


	
}