uniform mat4 imageMatrix;
uniform mat4 brushMatrix;
uniform mat4 textureMatrix;

out vec2 vertexPos;
out vec2 texturePos;

struct StrokePoint {
    vec2 pos;
    float pressure;
};

const int STROKE_SIZE_MAX = 4096;
layout(std140) uniform Stroke {
    StrokePoint stroke[STROKE_SIZE_MAX];
};

const vec2 vertices[4] = vec2[](
    vec2(-1.0,  1.0),
    vec2(-1.0, -1.0),
    vec2( 1.0,  -1.0),
    vec2( 1.0, 1.0)
);

void main(void)
{
    StrokePoint point = stroke[gl_InstanceID];
    vec2 vertex = vertices[gl_VertexID];
    vec4 imageSpaceVertex = brushMatrix * vec4(vertex, 0.0, 1.0) + vec4(point.pos, 0.0, 0.0);
    gl_Position = imageMatrix * imageSpaceVertex;
    vertexPos = vertex;
    texturePos = vec2(textureMatrix * vec4(vertex, 0.0, 1.0)).xy;
}
