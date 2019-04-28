varying vec2 vertexPos;
varying vec2 texturePos;

uniform sampler2D imageTypeTexture;
uniform int type;
uniform vec4 colour;
uniform float opacity;
uniform float hardness;
uniform bool singleColour;
uniform sampler2D paletteTexture;
uniform int paletteTextureSize;

// Brush::Type enum
const int SquareType = 0;
const int CircleType = 1;
const int ImageType = 2;

float lengthManhattan(const vec2 pos) {
    return abs(pos.x) + abs(pos.y);
}

float lengthChebyshev(const vec2 pos) {
    return max(abs(pos.x), abs(pos.y));
}

float applyHardness(const float hardness, const float weight) {
    return hardness < 1.0 ? clamp(weight / (1.0 - hardness), 0.0, 1.0) : 1.0;
}

void main(void)
{
    float dist = (type == CircleType ? length(vertexPos) : lengthChebyshev(vertexPos));
    if (dist > 1.0) discard;
    vec3 rgb = ((type == ImageType && !singleColour) ? texture2D(imageTypeTexture, texturePos).rgb : colour.rgb);
    float a = (type == ImageType ? texture2D(imageTypeTexture, texturePos).a * (singleColour ? colour.a : 1.0) : colour.a * applyHardness(hardness, 1.0 - dist));
    gl_FragColor = vec4(rgb, a * opacity);
    gl_FragDepth = dist;
}
