#version 410

in vec4 vPosition;
out vec4 color;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform float uniformTime;
uniform vec4 uniformColor;

void main()
{
   gl_Position = Projection * ModelView * vPosition; //+ uniformTime*.001;
   color = vec4(1,0,0,1);
   color = uniformColor;
}
