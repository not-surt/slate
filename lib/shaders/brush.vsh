attribute highp vec2 vertex;

uniform highp mat4 matrix;

varying highp vec2 texturePos;

void main(void)
{
    gl_Position = matrix * vec4(vertex, 0.0, 1.0);
    texturePos = vertex;
}
