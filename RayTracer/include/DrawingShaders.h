#include <glad/glad.h>

#ifndef STRINGIFY
#define STRINGIFY(A)  #A
#endif // 

static const GLchar* rayTraceVS = STRINGIFY(
\n#version 430 core\n

in vec2 aPos;
in vec2 aTexCoords;

out vec2 io_texCoord;

void main()
{
	gl_Position = vec4(aPos, 0.0, 1.0);
	io_texCoord = aTexCoords;
}
);

static const GLchar* rayTraceFS = STRINGIFY(
\n#version 430 core\n

in vec2 io_texCoord;

uniform sampler2D tex;
layout(location = 0) out vec4 color;
void main(void) {
	color = texture(tex, io_texCoord);
}
);