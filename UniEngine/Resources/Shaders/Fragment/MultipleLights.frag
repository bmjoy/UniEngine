out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpaces[DIRECTIONAL_LIGHTS_AMOUNT];
} fs_in;

void main()
{    
    // properties
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(CameraPosition - fs_in.FragPos);

    vec3 color = texture(TEXTURE_DIFFUSE[0], fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.3 * color;



    float directionalLightshadow = DirectionalLightShadowCalculation(fs_in.FragPosLightSpaces, norm);
    float pointLightShadow = PointLightShadowCalculation(fs_in.FragPos);

    vec3 result = CalculateLights(norm, viewDir, fs_in.FragPos);

    float lightness = 1.0 - directionalLightshadow - pointLightShadow;
    if(lightness < 0.0) lightness = 0.0;
    FragColor = vec4((ambient + lightness * result) * color, 1.0);
}
