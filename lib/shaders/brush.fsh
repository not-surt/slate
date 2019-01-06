varying highp vec2 texturePos;

uniform highp sampler2D imageTypeTexture;
uniform highp uvec2 imageTypeTextureSize;
uniform highp int type;
uniform highp vec4 colour;
uniform highp float opacity;
uniform highp float hardness;
uniform highp bool singleColour;
uniform highp sampler2D paletteTexture;
uniform highp uint paletteTextureSize;

// Brush::Type enum
const int SquareType = 0;
const int CircleType = 1;
const int ImageType = 2;

vec4 premultiply(vec4 colour) {
    return vec4(colour.rgb * colour.a, colour.a);
}

vec4 unpremultiply(const vec4 colour) {
    if (colour.a == 0.0) return colour;
    else return vec4(colour.rgb / colour.a, colour.a);
}

float lengthManhattan(const vec2 pos) {
    return abs(pos.x) + abs(pos.y);
}

float lengthChebyshev(const vec2 pos) {
    return max(abs(pos.x), abs(pos.y));
}

void main(void)
{
    float dist = (type == CircleType ? length(texturePos) : lengthChebyshev(texturePos));
    if (dist > 1.0) discard;
    vec3 rgb = ((type == ImageType && !singleColour) ? texture2D(imageTypeTexture, texturePos).rgb : colour.rgb);
    float a = ((type == ImageType) ? texture2D(imageTypeTexture, texturePos).a * colour.a : colour.a * (hardness < 1.0 ? clamp((1.0 - dist) / (1.0 - hardness), 0.0, 1.0) : 1.0));
    gl_FragColor = premultiply(vec4(rgb, a * opacity));
}
