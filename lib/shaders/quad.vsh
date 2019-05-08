uniform mat4 vertexMatrix;
uniform mat4 textureMatrix;

out vec2 vertexPos;
out vec2 texturePos;

const vec2 vertices[4] = vec2[](
    vec2(-1.0,  1.0),
    vec2(-1.0, -1.0),
    vec2( 1.0,  -1.0),
    vec2( 1.0, 1.0)
);

void main(void)
{
    vec2 vertex = vertices[gl_VertexID];
    gl_Position = vertexMatrix * vec4(vertex, 0.0, 1.0);
    vertexPos = vertex;
    texturePos = vec2(textureMatrix * vec4(vertex, 0.0, 1.0)).xy;
}
