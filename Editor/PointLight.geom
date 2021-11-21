#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout (set = 0, binding = 0) uniform CamMatrix{
    vec3 light_position;
    mat4 Projection;
}view_proj_mat;

out gl_PerVertex
{
  vec4 gl_Position;
};

/*
layout (location = 0) in VertexData
{
  vec4 Position;
} inData[];
*/

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        for(int i = 0; i < 3; ++i) // for each triangle vertex
        {
            gl_Layer = face; // built-in variable that specifies to which face we render.
            vec3 up;
            vec3 forward;
            switch(face)
            {
            // Positive X
            case 0:
            {
                forward = vec3( 1.0, 0.0, 0.0);
                up = vec3(0.0, -1.0,0.0);
                break;
            }
            // Negative X
            case 1:
            {
                forward = vec3(-1.0, 0.0, 0.0);
                up = vec3(0.0, -1.0,0.0);
                break;
            }
            // Positive Y
            case 2:
            {
                // out Y axis is inverted
                forward = vec3( 0.0, 1.0, 0.0);
                up = vec3(0.0, 0.0,1.0);
                break;
            }
            // Negative Y
            case 3:
            {
                // out Y axis is inverted
                forward = vec3( 0.0,-1.0, 0.0);
                up = vec3(0.0, 0.0,-1.0);
                break;
            }
            // Positive Z
            case 4:
            {
                forward = vec3( 0.0, 0.0, 1.0);
                up = vec3(0.0, 1.0, 0.0);
                break;
            }
            // Negative Z
            case 5:
            {
                forward = vec3( 0.0, 0.0,-1.0);
                up = vec3(0.0,-1.0, 0.0);
                break;
            }
            }

            vec3 s = vec3(normalize(cross(forward, up)));
            vec3 u = vec3(cross(s, forward));

            mat4 lookAtMatrix;
            lookAtMatrix[0][0] = s.x;
            lookAtMatrix[1][0] = s.y;
            lookAtMatrix[2][0] = s.z;
            lookAtMatrix[0][1] = u.x;
            lookAtMatrix[1][1] = u.y;
            lookAtMatrix[2][1] = u.z;
            lookAtMatrix[0][2] =-forward.x;
            lookAtMatrix[1][2] =-forward.y;
            lookAtMatrix[2][2] =-forward.z;
            lookAtMatrix[3][0] =-dot(s, view_proj_mat.light_position);
            lookAtMatrix[3][1] =-dot(u, view_proj_mat.light_position);
            lookAtMatrix[3][2] = dot(forward, view_proj_mat.light_position);


            lookAtMatrix[0][3] = 0;
            lookAtMatrix[1][3] = 0;
            lookAtMatrix[2][3] = 0;
            lookAtMatrix[3][3] = 1;

            //gl_Position = view_proj_mat.Projection * lookAtMatrix * inData[i].Position;
            gl_Position = view_proj_mat.Projection * lookAtMatrix * gl_in[i].gl_Position;
            EmitVertex();
        }    
        EndPrimitive();
    }
}