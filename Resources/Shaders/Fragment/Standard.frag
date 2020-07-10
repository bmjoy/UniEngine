
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

vec3 CalculateLights(float distance, vec3 normal, vec3 viewDir, vec3 fragPos);


vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);




float DirectionalLightShadowCalculation(int i, int splitIndex, DirectionalLight light, vec4 fragPosLightSpace, vec3 normal);
float PointLightShadowCalculation(int i, PointLight light, vec3 fragPos, vec3 normal);



void main()
{    
    // properties
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(CameraPosition - fs_in.FragPos);

    vec3 color = texture(TEXTURE_DIFFUSE[0], fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;

    float distance = distance(fs_in.FragPos, CameraPosition);

    if(DisplaySplit != 0){
        if(distance < SplitDistance0){
            ambient += vec3(0.0, 0.0, 0.2);
        }else if(distance < SplitDistance1){
            ambient += vec3(0.2, 0.0, 0.0);
        }else if(distance < SplitDistance2){
            ambient += vec3(0.0, 0.2, 0.0);
        }else if(distance < SplitDistance3){
        }
    }
    
    vec3 result = CalculateLights(distance, norm, viewDir, fs_in.FragPos);

    FragColor = vec4((ambient + result) * color, 1.0);
}



vec3 CalculateLights(float distance, vec3 normal, vec3 viewDir, vec3 fragPos){
    vec3 norm = normalize(normal);
    vec3 result = vec3(0.0, 0.0, 0.0);

    // phase 1: directional lighting
    for(int i = 0; i < DirectionalLightCount; i++){
        int split = 0;
        float shadow = 0.0;
        if(distance < SplitDistance0 - SplitDistance0 * SeamFixRatio){
            shadow = DirectionalLightShadowCalculation(i, 0, DirectionalLights[i], DirectionalLights[i].lightSpaceMatrix[0] * vec4(fs_in.FragPos, 1.0), norm);
        }else if(distance < SplitDistance0){
            //Blend between split 1 & 2
            shadow = DirectionalLightShadowCalculation(i, 0, DirectionalLights[i], DirectionalLights[i].lightSpaceMatrix[0] * vec4(fs_in.FragPos, 1.0), norm);
            float nextLevel = DirectionalLightShadowCalculation(i, 1, DirectionalLights[i], DirectionalLights[i].lightSpaceMatrix[1] * vec4(fs_in.FragPos, 1.0), norm);
            shadow = (nextLevel * (distance - (SplitDistance0 - SplitDistance0 * SeamFixRatio)) + shadow * (SplitDistance0 - distance)) / (SplitDistance0 * SeamFixRatio);
        }else if(distance < SplitDistance1 - SplitDistance1 * SeamFixRatio){
            shadow = DirectionalLightShadowCalculation(i, 1, DirectionalLights[i], DirectionalLights[i].lightSpaceMatrix[1] * vec4(fs_in.FragPos, 1.0), norm);
        }else if(distance < SplitDistance1){
            //Blend between split 2 & 3
            shadow = DirectionalLightShadowCalculation(i, 1, DirectionalLights[i], DirectionalLights[i].lightSpaceMatrix[1] * vec4(fs_in.FragPos, 1.0), norm);
            float nextLevel = DirectionalLightShadowCalculation(i, 2, DirectionalLights[i], DirectionalLights[i].lightSpaceMatrix[2] * vec4(fs_in.FragPos, 1.0), norm);
            shadow = (nextLevel * (distance - (SplitDistance1 - SplitDistance1 * SeamFixRatio)) + shadow * (SplitDistance1 - distance)) / (SplitDistance1 * SeamFixRatio);
        }else if(distance < SplitDistance2 - SplitDistance2 * SeamFixRatio){
            shadow = DirectionalLightShadowCalculation(i, 2, DirectionalLights[i], DirectionalLights[i].lightSpaceMatrix[2] * vec4(fs_in.FragPos, 1.0), norm);
        }else if(distance < SplitDistance2){
            //Blend between split 3 & 4
            shadow = DirectionalLightShadowCalculation(i, 2, DirectionalLights[i], DirectionalLights[i].lightSpaceMatrix[2] * vec4(fs_in.FragPos, 1.0), norm);
            float nextLevel = DirectionalLightShadowCalculation(i, 3, DirectionalLights[i], DirectionalLights[i].lightSpaceMatrix[3] * vec4(fs_in.FragPos, 1.0), norm);
            shadow = (nextLevel * (distance - (SplitDistance2 - SplitDistance2 * SeamFixRatio)) + shadow * (SplitDistance2 - distance)) / (SplitDistance2 * SeamFixRatio);
        }else if(distance < SplitDistance3){
            shadow = DirectionalLightShadowCalculation(i, 3, DirectionalLights[i], DirectionalLights[i].lightSpaceMatrix[3] * vec4(fs_in.FragPos, 1.0), norm);
        }
        
        result += CalcDirectionalLight(DirectionalLights[i], norm, viewDir) * shadow;
    }
    // phase 2: point lights
    for(int i = 0; i < PointLightCount; i++){
        float shadow = PointLightShadowCalculation(i, PointLights[i], fragPos, norm);
        result += CalcPointLight(PointLights[i], norm, fragPos, viewDir) * (1.0 - shadow);
    }
    // phase 3: spot light
    for(int i = 0; i < SpotLightCount; i++){
        result += CalcSpotLight(SpotLights[i], norm, fragPos, viewDir);
    }
    return result;
}

// calculates the color when using a directional light.
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // combine results
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    return (diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constantLinearQuadFarPlane.x + light.constantLinearQuadFarPlane.y * distance + light.constantLinearQuadFarPlane.z * (distance * distance));    
    // combine results
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    diffuse *= attenuation;
    specular *= attenuation;
    return (diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (diffuse + specular);
}



// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float Linstep(float minVal, float maxVal, float val) {
	return clamp((val - minVal) / (maxVal - minVal), 0.0, 1.0);
}
float ReduceLightBleed(float p_max, float amount) {
	return Linstep(amount, 1.0, p_max);
}

float Chebyshev(vec2 moments, float depth)
{
    if (depth <= moments.x) {
		return 1.0;
	}

	float variance = moments.y - (moments.x * moments.x);
	variance = max(variance, VSMMaxVariance / 1000.0);

	float d = depth - moments.x; // attenuation
	float p_max = variance / (variance + d * d);

	return ReduceLightBleed(p_max, LightBleedFactor);
}

float DirectionalLightShadowCalculation(int i, int splitIndex, DirectionalLight light, vec4 fragPosLightSpace, vec3 normal)
{
    vec3 lightDir = light.direction;

    float bias = light.ReservedParameters.z;

    //float bias = 0.005 * tan(acos(clamp(dot(normal, lightDir), 0.0, 1.0)));
    //bias = clamp(bias, 0.0, 0.02);

    float normalOffset = light.ReservedParameters.w;

    // perform perspective divide
    vec3 projCoords = (fragPosLightSpace.xyz + normal * normalOffset) / fragPosLightSpace.w;
    //
    if(projCoords.z > 1.0){
        return 0.0;
    }
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z - bias;
    float shadow = 0.0;
    if(light.ReservedParameters.y != 0){
        // check whether current frag pos is in shadow
        // PCF
        if(EnableVSM != 0){
            if(EnableEVSM != 0){
                vec4 moments = texture(directionalShadowMap, vec3(projCoords.xy, i * 4 + splitIndex));
                float depth = currentDepth * 2.0 - 1.0;
                float pos = exp(EVSMExponent * depth);
                float neg = -exp(-EVSMExponent * depth);
                float posShadow = Chebyshev(moments.xy, pos);
		        float negShadow = Chebyshev(moments.zw, neg);
		        shadow = min(posShadow, negShadow);
            }else{
                float texelSize = 0.1 / textureSize(directionalShadowMap, 0).x;
                for(int x = -0; x <= 0; ++x)
                {
                    for(int y = -0; y <= 0; ++y)
                    {
                        vec3 moments = texture(directionalShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, i * 4 + splitIndex)).rgb;
                        shadow += Chebyshev(moments.xy, currentDepth);
                    }    
                }
                //shadow /= 9.0;
            }
        }else{
            float texelSize = 0.2 / textureSize(directionalShadowMap, 0).x;
            for(int x = -3; x <= 3; ++x)
            {
                for(int y = -3; y <= 3; ++y)
                {
                    float cloestDepth = texture(directionalShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, i * 4 + splitIndex)).r;
                    shadow += currentDepth <= cloestDepth ? 1.0 : 0.0; 
                }    
            }
            shadow /= 49.0;
        }
    }else{
        float cloestDepth = texture(directionalShadowMap, vec3(projCoords.xy, i * 4 + splitIndex)).r;
        shadow += currentDepth <= cloestDepth ? 1.0 : 0.0; 
    }
    return shadow;
}

float PointLightShadowCalculation(int i, PointLight light, vec3 fragPos, vec3 normal)
{
    vec3 viewPos = CameraPosition;
    vec3 lightPos = light.position;
    float far_plane = light.constantLinearQuadFarPlane.w;
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    if(currentDepth > far_plane) return 0.0;
    float shadow = 0.0;
    float bias = light.ReservedParameters.x;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    float compare = 0.0;
    for(int j = 0; j < samples; ++j)
    {
        shadow += texture(pointShadowMap, vec4((fragToLight + gridSamplingDisk[j] * diskRadius), i), (currentDepth - bias) / far_plane, 0);
    }
    shadow /= float(samples);
    return shadow;
}
