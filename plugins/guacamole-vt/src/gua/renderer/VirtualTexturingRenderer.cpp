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

// class header
#include <gua/renderer/VirtualTexturingRenderer.hpp>
#include <gua/renderer/VirtualTexturingPass.hpp>

// guacamole headers
#include <gua/renderer/Pipeline.hpp>
#include <gua/renderer/ShaderProgram.hpp>
#include <gua/renderer/GBuffer.hpp>
#include <gua/renderer/ResourceFactory.hpp>

#include <gua/platform.hpp>
#include <gua/guacamole.hpp>
#include <gua/renderer/View.hpp>

#include <gua/databases.hpp>
#include <gua/utils/Logger.hpp>

#include <gua/config.hpp>
#include <scm/gl_core/shader_objects.h>

// external headers
#include <sstream>
#include <fstream>
#include <regex>
#include <list>

/*
#include <lamure/ren/camera.h>
#include <lamure/ren/policy.h>
#include <lamure/ren/dataset.h>
#include <lamure/ren/model_database.h>
#include <lamure/ren/cut_database.h>
#include <lamure/ren/controller.h>
*/

#include <boost/assign/list_of.hpp>

namespace gua {



  //////////////////////////////////////////////////////////////////////////////
  VirtualTexturingRenderer::VirtualTexturingRenderer() 
  {}


  ///////////////////////////////////////////////////////////////////////////////
  void VirtualTexturingRenderer::_create_gpu_resources(gua::RenderContext const& ctx,
                                           scm::math::vec2ui const& render_target_dims) {
    //invalidation before first write
    previous_frame_count_ = UINT_MAX;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////

/*
  lamure::context_t PLodRenderer::_register_context_in_cut_update(gua::RenderContext const& ctx) {
    lamure::ren::controller* controller = lamure::ren::controller::get_instance(); 
    if (previous_frame_count_ != ctx.framecount) {
      controller->reset_system();
    }
    return controller->deduce_context_id(ctx.id);
  }
*/


  /////////////////////////////////////////////////////////////////////////////////////////////
  void VirtualTexturingRenderer::set_global_substitution_map(SubstitutionMap const& smap) {

  }

  ///////////////////////////////////////////////////////////////////////////////
  void VirtualTexturingRenderer::render(gua::Pipeline& pipe, PipelinePassDescription const& desc) {

    RenderContext const& ctx(pipe.get_context());

    ///////////////////////////////////////////////////////////////////////////
    //  retrieve current view state
    ///////////////////////////////////////////////////////////////////////////
    auto& scene = *pipe.current_viewstate().scene;
    auto const& camera = pipe.current_viewstate().camera;
    auto const& frustum = pipe.current_viewstate().frustum;
    auto& target = *pipe.current_viewstate().target;

    std::string cpu_query_name_plod_total = "CPU: Camera uuid: " + std::to_string(pipe.current_viewstate().viewpoint_uuid) + " / LodPass";
    pipe.begin_cpu_query(cpu_query_name_plod_total);
    


    std::cout << "Render Frame in VT-Renderer\n";

    pipe.end_cpu_query(cpu_query_name_plod_total); 
    
    //dispatch cut updates
    if (previous_frame_count_ != ctx.framecount) {
      previous_frame_count_ = ctx.framecount;
      //controller->dispatch(controller->deduce_context_id(ctx.id), ctx.render_device);
    }
  } 

}
