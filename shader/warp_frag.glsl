/*%%HEADER%%*/

precision highp float;
precision highp int;
precision highp image2D;

uniform sampler2D colorTexture;
uniform sampler2D xyzTexture;
uniform mat4 currentViewMatrix;

layout(binding = 0, rgba32f) uniform highp writeonly image2D outputColorImage;
layout(binding = 1, rgba32f) uniform highp writeonly image2D outputXYZImage;

in vec2 uv;

void main() {
  vec3 worldCoords = texture(xyzTexture, uv).rgb;
  vec4 newCoords = currentViewMatrix * vec4(worldCoords, 1.0);
  vec3 projectedCoords = newCoords.xyz / newCoords.w;
  vec2 newUV = projectedCoords.xy * 0.5 + 0.5;

  if (newUV.x >= 0.0 && newUV.x <= 1.0 && newUV.y >= 0.0 && newUV.y <= 1.0) {
    vec4 warpedColor = texture(colorTexture, uv);
    
    // 3. Cast imageSize (ivec2) to vec2 to prevent strict type-multiplication errors
    ivec2 writeCoords = ivec2(newUV * vec2(imageSize(outputColorImage)));
    
    imageStore(outputColorImage, writeCoords, warpedColor);
    imageStore(outputXYZImage, writeCoords, vec4(worldCoords, 0.0));
  } 
}