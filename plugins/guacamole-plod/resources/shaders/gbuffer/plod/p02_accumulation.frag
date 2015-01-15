@include "resources/shaders/common/header.glsl"

const float gaussian[32] = float[](
  1.000000, 1.000000, 0.988235, 0.968627, 0.956862, 0.917647, 0.894117, 0.870588, 0.915686, 0.788235,
  0.749020, 0.690196, 0.654902, 0.619608, 0.552941, 0.513725, 0.490196, 0.458824, 0.392157, 0.356863,
  0.341176, 0.278431, 0.254902, 0.227451, 0.188235, 0.164706, 0.152941, 0.125490, 0.109804, 0.098039,
  0.074510, 0.062745
);

///////////////////////////////////////////////////////////////////////////////
// input
///////////////////////////////////////////////////////////////////////////////
in VertexData {
  vec3 pass_point_color;
  vec3 pass_normal;
  vec2 pass_uv_coords;
  float pass_es_linear_depth;
} VertexIn;

///////////////////////////////////////////////////////////////////////////////
// output
///////////////////////////////////////////////////////////////////////////////

layout (location = 0) out vec4 out_accumulated_color;
layout (location = 1) out vec4 out_accumulated_normal;

///////////////////////////////////////////////////////////////////////////////
//sampler
///////////////////////////////////////////////////////////////////////////////
layout(binding=0) uniform sampler2D p01_depth_texture;

///////////////////////////////////////////////////////////////////////////////
// splatting methods
///////////////////////////////////////////////////////////////////////////////
uniform vec2 win_dims;
uniform float far_plane;
///////////////////////////////////////////////////////////////////////////////
// main
///////////////////////////////////////////////////////////////////////////////
void main() {

  float pass_1_linear_depth = -far_plane*(texture2D(p01_depth_texture, gl_FragCoord.xy/win_dims).r);

  //if( gl_FragCoord.z - gl_FragCoord.z > 0.001 ) {
  //  discard;
 // }

  if(pass_1_linear_depth > -5.0)
    discard;

  vec2 uv_coords = VertexIn.pass_uv_coords;


  //if( (uv_coords.x * uv_coords.x + uv_coords.y * uv_coords.y) > 1)
  //  discard;
 

  if(pass_1_linear_depth < 0.0)
    discard;

  float normalAdjustmentFactor = 1.0;

  if (VertexIn.pass_normal.z < 0.0) {
    normalAdjustmentFactor = -1.0;
  }



  float weight = gaussian[(int)(round(length(uv_coords) * 31.0))];

   // if( !(render_target_width > 1900.0 && render_target_height > 1000.0 && render_target_width < 2000.0 && render_target_height < 1200) )
      //out_accumulated_color = vec4(1.0, 0.0, 0.0, 1.0);
   // else
  out_accumulated_color = vec4(VertexIn.pass_point_color * weight, weight);
    //out_accumulated_color = vec4(u, v, 0.0, 1.0);//vec4(pass_normal.xyz, 1.0);
   // out_accumulated_color = vec4(-q_n, 1.0);
  out_accumulated_normal = normalAdjustmentFactor * vec4(weight * VertexIn.pass_normal, weight);

}

