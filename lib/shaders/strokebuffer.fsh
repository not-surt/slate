precision highp float;
precision highp int;

varying vec2 vertexPos;
varying vec2 texturePos;

uniform sampler2D texture;

vec4 premultiply(const vec4 colour) {
    return vec4(colour.rgb * colour.a, colour.a);
}

vec4 unpremultiply(const vec4 colour) {
    if (colour.a == 0.0) return colour;
    else return vec4(colour.rgb / colour.a, colour.a);
}

void main(void)
{
    gl_FragColor = premultiply(texture2D(texture, texturePos));
}
