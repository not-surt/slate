attribute highp vec2 vertex;

uniform highp mat4 matrix;
uniform highp mat4 textureMatrix;

varying highp vec2 vertexPos;
varying highp vec2 texturePos;

void main(void)
{
    gl_Position = matrix * vec4(vertex, 0.0, 1.0);
    vertexPos = vertex;
    texturePos = vec2(textureMatrix * vec4(vertex, 0.0, 1.0)).xy;
}
