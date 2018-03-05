/******************************************************************************
 * guacamole - delicious VR                                                   *
 *                                                                            *
 * Copyright: (c) 2011-2013 Bauhaus-Universität Weimar                        *
 * Contact:   felix.lauer@uni-weimar.de / simon.schneegans@uni-weimar.de      *
 *                                                                            *
 * This program is free software: you can redistribute it and/or modify it    *
 * under the terms of the GNU General Public License as published by the Free *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   *
 * for more details.                                                          *
 *                                                                            *
 * You should have received a copy of the GNU General Public License along    *
 * with this program. If not, see <http://www.gnu.org/licenses/>.             *
 *                                                                            *
 ******************************************************************************/

#ifndef GUA_VIRTUAL_TEXTURING_RENDERER_HPP
#define GUA_VIRTUAL_TEXTURING_RENDERER_HPP

#include <string>
#include <map>
#include <unordered_map>

// guacamole headers
#include <gua/renderer/VirtualTexturingPass.hpp>

#include <gua/renderer/Pipeline.hpp>
#include <gua/renderer/View.hpp>
#include <gua/renderer/ShaderProgram.hpp>
#include <gua/renderer/ResourceFactory.hpp>

namespace gua {

  class MaterialShader;
  class ShaderProgram;
  //class plod_shared_resources;

  class GUA_VT_DLL VirtualTexturingRenderer {
 
  public:

    VirtualTexturingRenderer();

    void render(Pipeline& pipe, PipelinePassDescription const& desc);
    void set_global_substitution_map(SubstitutionMap const& smap);

  private:  //shader related auxiliary methods
    void          _create_gpu_resources(gua::RenderContext const& ctx,
                                        scm::math::vec2ui const& render_target_dims); 
    
    void          _check_for_resource_updates(gua::Pipeline const& pipe, RenderContext const& ctx);

  private:  //out-of-core related auxiliary methods
  
  private:  //member variables
    unsigned                                                                          previous_frame_count_;

    //CPU resources
    SubstitutionMap                                                                   global_substitution_map_;
    ResourceFactory                                                                   factory_;

  };

}

#endif  // GUA_P_LOD_RENDERER_HPP
