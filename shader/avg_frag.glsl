/*%%HEADER%%*/

precision highp float;
precision highp int;
precision highp image2D;

uniform sampler2D currentColorTexture;  // Current color texture
uniform sampler2D currentDepthTexture;  // Current depth texture (normalized depth)
uniform sampler2D warpedColorTexture;  // Warped color texture
uniform sampler2D warpedXYZTexture;    // Warped XYZ texture (world coordinates)
uniform bool viewChanged;
uniform mat4 invProjViewMat;

in vec2 uv;                            // Interpolated UV coordinates
layout(location = 0) out vec4 outColor;  // Output color
layout(location = 1) out vec4 outXYZ;  // Output world coordinates

void main() {
  float depth = texture(currentDepthTexture, uv).r;
  float zClip = depth * 2.0 - 1.0;
  vec4 worldCoords = invProjViewMat * vec4(2.0f * uv - 1.0f, zClip, 1.0);
  worldCoords /= worldCoords.w;
  vec3 warpedXYZ = texture(warpedXYZTexture, uv).rgb;
  vec4 warpedColor = texture(warpedColorTexture, uv);
  float distance = length(worldCoords.xyz - warpedXYZ);
  vec4 currentColor = texture(currentColorTexture, uv);

  // Check if we have valid warped data (alpha > 0 means warp shader wrote to this pixel)
  bool hasValidWarpedData = warpedColor.w > 0.0;
  bool shouldUseTemporalData = !viewChanged || (hasValidWarpedData && distance < 100.0f);
  
  if (shouldUseTemporalData) {
    float alpha = min(warpedColor.w, 128.0);
    outXYZ.xyz = (alpha / (alpha + 1.0f)) * warpedXYZ.xyz +
                 (1.0f / (alpha + 1.0f)) * worldCoords.xyz;
    outColor.xyz = (alpha / (alpha + 1.0f)) * warpedColor.xyz +
                   (1.0f / (alpha + 1.0f)) * currentColor.xyz;
    outColor.w = min(alpha + 1.0f, 128.0f);
  } else {
    // No valid temporal data - start fresh with current frame
    outXYZ = worldCoords;
    outColor = currentColor;
    outColor.w = 1.0f;  // Start accumulation from 1
  }
}