in vec2 vertexPos;
in vec2 texturePos;

uniform sampler2D texture;

layout (location = 0) out vec4 fragColour;

vec4 unpremultiply(const vec4 colour) {
    if (colour.a == 0.0) return colour;
    else return vec4(colour.rgb / colour.a, colour.a);
}

void main(void)
{
    fragColour = unpremultiply(texture2D(texture, texturePos));
}
