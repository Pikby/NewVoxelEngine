
#version 450

out vec4 FragColor;
in vec3 Norm;
in vec4 Color;
in vec3 WorldPos;
in vec4 DirectionalShadowPos;

uniform float time;
uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform vec3 diffuseColor;

uniform float specularExponent;

uniform vec3 globalLightDir;
uniform vec3 cameraPos;
uniform int translucent;

uniform sampler2D waterTexture;
uniform sampler2D shadowTexture;


	float bias = 0.005;

float chebyshevUpperBound(vec4 shadowPos)
{
	vec2 moments = texture2D(shadowTexture,shadowPos.xy).rg;
	
	// Surface is fully lit. as the current fragment is before the light occluder
	if (shadowPos.z <= moments.x)
		return 1.0;

	// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
	// How likely this pixel is to be lit (p_max)
	float variance = moments.y - (moments.x*moments.x);
	//variance = max(variance, 0.000002);
	variance = max(variance, 0.000002);

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
	vec2 texelSize = 1.0 / textureSize(shadowTexture, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowTexture, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;
	/*


	float closestDepth = texture(shadowTexture, projCoords.xy).r;  
 
	

	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  
	*/
	return shadow;
}



vec3 calculateGlobalColor(){
	vec3 diffuse = max(dot(Norm, globalLightDir),0.0)*diffuseColor;
	vec3 viewDir = normalize(cameraPos-WorldPos);
	vec3 reflectDir = reflect(-globalLightDir,Norm);
	float spec = pow(max(dot(viewDir,reflectDir),0),specularExponent);
	vec3 specular = 0.5*spec*specularColor;

	
	vec4 scPostW = DirectionalShadowPos/DirectionalShadowPos.w; 
	scPostW = scPostW * 0.5 + 0.5;
	

	float shadow = 1.0; // Not in shadow
	//shadow = shadowCalc(DirectionalShadowPos);
	
	
	bool outsideShadowMap = DirectionalShadowPos.w <= 0.0f || (scPostW.x < 0 || scPostW.y < 0) || (scPostW.x >= 1 || scPostW.y >= 1);
	if (!outsideShadowMap) 
	{
		shadow = 1-chebyshevUpperBound(scPostW);
	}
	
	return (ambientColor+(1-shadow))*(diffuse+specular);
}

void main() {



	vec3 globalColor = calculateGlobalColor();

	
	FragColor = vec4((globalColor)*Color.rgb,Color.a);

	if(translucent == 1){
		vec2 textureOrigin = cameraPos.xz*4-vec2(512);
		vec2 textureCoord = ( WorldPos.xz - textureOrigin);
			FragColor = FragColor*(max((texture(waterTexture,((WorldPos.xz +vec2(time*10))/1024.0f)).r+0.5),1));
	}

	
}