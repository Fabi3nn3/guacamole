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
#include <gua/renderer/VTTexture2D.hpp>

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

#include <lamure/vt/ren/CutUpdate.h>
#include <lamure/vt/ren/CutDatabase.h>
#include <lamure/vt/common.h>
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

  
  void VirtualTexturingRenderer::apply_cutupdate(uint16_t ctx_id){
      std::cout << "Context id: " << ctx_id << std::endl;
      vt::CutDatabase *cut_db = &vt::CutDatabase::get_instance();
      cut_db->start_reading();

      for(vt::cut_map_entry_type cut_entry : (*cut_db->get_cut_map()))
      {
        vt::Cut *cut = cut_db->start_reading_cut(cut_entry.first);

        if(!cut->is_drawn())
        {
            cut_db->stop_reading_cut(cut_entry.first);
            continue;
        }
        //TODO
        //update_index_texture(Cut::get_dataset_id(cut_entry.first), ctx_id, cut->get_front()->get_index());
        for(auto position_slot_updated : cut->get_front()->get_mem_slots_updated())
        {
          const vt::mem_slot_type *mem_slot_updated = &cut_db->get_front()->at(position_slot_updated.second);

          if(mem_slot_updated == nullptr || !mem_slot_updated->updated || !mem_slot_updated->locked || mem_slot_updated->pointer == nullptr)
          {
               if(mem_slot_updated == nullptr)
               {
                   std::cerr << "Mem slot at " << position_slot_updated.second << " is null" << std::endl;
               }
               else
               {
                   std::cerr << "Mem slot at " << position_slot_updated.second << std::endl;
                   std::cerr << "Mem slot #" << mem_slot_updated->position << std::endl;
                   std::cerr << "Tile id: " << mem_slot_updated->tile_id << std::endl;
                   std::cerr << "Locked: " << mem_slot_updated->locked << std::endl;
                   std::cerr << "Updated: " << mem_slot_updated->updated << std::endl;
                   std::cerr << "Pointer valid: " << (mem_slot_updated->pointer != nullptr) << std::endl;
               }
               throw std::runtime_error("updated mem slot inconsistency");
          }
          //TODO
          //update_physical_texture_blockwise(ctx_id, mem_slot_updated->pointer, mem_slot_updated->position);

        }
        cut_db->stop_reading_cut(cut_entry.first);
        
      }
      cut_db->stop_reading();
      //TODO
      //_context_resources[ctx_id]->_render_context->sync();
  }


  ///////////////////////////////////////////////////////////////////////////////
  void VirtualTexturingRenderer::render(gua::Pipeline& pipe, PipelinePassDescription const& desc) {

    RenderContext const& ctx(pipe.get_context());

    auto scm_device = ctx.render_device;
    auto scm_context = ctx.render_context;


    ///////////////////////////////////////////////////////////////////////////
    //  retrieve current view state
    ///////////////////////////////////////////////////////////////////////////
    auto& scene = *pipe.current_viewstate().scene;
    auto const& camera = pipe.current_viewstate().camera;
    auto const& frustum = pipe.current_viewstate().frustum;
    auto& target = *pipe.current_viewstate().target;

    std::string cpu_query_name_plod_total = "CPU: Camera uuid: " + std::to_string(pipe.current_viewstate().viewpoint_uuid) + " / LodPass";
    pipe.begin_cpu_query(cpu_query_name_plod_total);
    

    // if(lamure_cut_update is not running -> run)

    //with lamure: start_reading_cut

    //with gua: get_front_idx (index texture cpu representation as uin8_t*) (gives you the lamure::vt::ren::Cut*)

    //with gua: get_physical texture mem slots from cut

    // either: update GPU representation directly similar to lamure, but with gua resources (in your VTTexture class)
    // OR: copy all the cpu memory you received into own CPU buffers in the VTTexture object

    // with lamure: stop reading cut

    // if you did not update the GPU textures before: update them now with the copied CPU representation





    //std::cout << "Render Frame in VT-Renderer\n";

    pipe.end_cpu_query(cpu_query_name_plod_total); 
    
    //dispatch cut updates
    if (previous_frame_count_ != ctx.framecount) {
      previous_frame_count_ = ctx.framecount;
      //controller->dispatch(controller->deduce_context_id(ctx.id), ctx.render_device);
    }
  } 

}
