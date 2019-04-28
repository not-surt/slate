precision highp float;
precision highp int;

attribute vec2 vertex;
attribute mat4 instanceTransform;

uniform mat4 vertexMatrix;
uniform mat4 textureMatrix;

varying vec2 vertexPos;
varying vec2 texturePos;

void main(void)
{
    gl_Position = vertexMatrix * vec4(vertex, 0.0, 1.0);
    vertexPos = vertex;
    texturePos = vec2(textureMatrix * vec4(vertex, 0.0, 1.0)).xy;
}
