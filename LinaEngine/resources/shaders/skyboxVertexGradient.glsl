
/*
 * Copyright (C) 2019 Inan Evin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include <common.glh>
#include <uniformBuffers.glh>

#if defined(VS_BUILD)
layout (location = 0) in vec3 position;

out vec3 rawPosition;
mat4 viewWOTranslation;

void main()
{
	viewWOTranslation = view;
	viewWOTranslation[3] = vec4(0,0,0,1.0);
    vec4 pos = projection * viewWOTranslation * vec4(position, 1.0);
    gl_Position = pos.xyww;
	rawPosition = position;
}
  
  
#elif defined(FS_BUILD)

in vec3 rawPosition;
out vec4 fragColor;

struct Material
{
vec3 startColor;
vec3 endColor;
};

uniform Material material;

void main()
{    
	float u = rawPosition.y;
	u = remap(u, -1.0f, 1.0f, 0.0f, 1.0f);
    gl_FragColor = mix( vec4(material.startColor, 1.0), vec4(material.endColor, 1.0), u );
}
#endif

