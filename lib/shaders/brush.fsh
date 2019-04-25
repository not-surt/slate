#extension GL_EXT_frag_depth : enable

varying highp vec2 vertexPos;
varying highp vec2 texturePos;

uniform highp sampler2D imageTypeTexture;
uniform highp int type;
uniform highp vec4 colour;
uniform highp float opacity;
uniform highp float hardness;
uniform bool singleColour;
uniform highp sampler2D paletteTexture;
uniform highp int paletteTextureSize;

// Brush::Type enum
const int SquareType = 0;
const int CircleType = 1;
const int ImageType = 2;

highp vec4 premultiply(const highp vec4 colour) {
    return vec4(colour.rgb * colour.a, colour.a);
}

highp vec4 unpremultiply(const highp vec4 colour) {
    if (colour.a == 0.0) return colour;
    else return vec4(colour.rgb / colour.a, colour.a);
}

highp float lengthManhattan(const highp vec2 pos) {
    return abs(pos.x) + abs(pos.y);
}

highp float lengthChebyshev(const highp vec2 pos) {
    return max(abs(pos.x), abs(pos.y));
}

highp float applyHardness(const highp float hardness, const highp float weight) {
    return hardness < 1.0 ? clamp(weight / (1.0 - hardness), 0.0, 1.0) : 1.0;
}

void main(void)
{
    highp float dist = (type == CircleType ? length(vertexPos) : lengthChebyshev(vertexPos));
    if (dist > 1.0) discard;
    highp vec3 rgb = ((type == ImageType && !singleColour) ? texture2D(imageTypeTexture, texturePos).rgb : colour.rgb);
    highp float a = (type == ImageType ? texture2D(imageTypeTexture, texturePos).a * (singleColour ? colour.a : 1.0) : colour.a * applyHardness(hardness, 1.0 - dist));
    gl_FragColor = premultiply(vec4(rgb, a * opacity));
//    gl_FragColor = vec4(rgb, a * opacity);
    gl_FragDepthEXT = dist;
}
