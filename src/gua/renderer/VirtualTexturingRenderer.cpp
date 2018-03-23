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
#include <lamure/vt/VTConfig.h>
#include <lamure/vt/common.h>

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
#include <scm/gl_core/render_device/context.h>

// external headers
#include <sstream>
#include <fstream>
#include <regex>
#include <list>



#include <lamure/ren/camera.h>
#include <lamure/ren/policy.h>
#include <lamure/ren/dataset.h>
#include <lamure/ren/model_database.h>
#include <lamure/ren/cut_database.h>
#include <lamure/ren/controller.h>


#include <lamure/vt/ren/CutUpdate.h>
#include <lamure/vt/ren/CutDatabase.h>


#include <boost/assign/list_of.hpp>

namespace gua {



  //////////////////////////////////////////////////////////////////////////////
  VirtualTexturingRenderer::VirtualTexturingRenderer()
  {

  }

  ///////////////////////////////////////////////////////////////////////////////
  void VirtualTexturingRenderer::_create_gpu_resources(gua::RenderContext const& ctx, uint64_t cut_id/*,
                                           scm::math::vec2ui const& render_target_dims*/) {}


  /////////////////////////////////////////////////////////////////////////////////////////////
  void VirtualTexturingRenderer::set_global_substitution_map(SubstitutionMap const& smap) {}
  
  void VirtualTexturingRenderer::apply_cut_update(gua::RenderContext const& ctx, uint64_t cut_id, uint16_t ctx_id){
      auto render_context = ctx.render_context;
      vt::CutDatabase *cut_db = &vt::CutDatabase::get_instance();
      cut_db->start_reading();

      for(vt::cut_map_entry_type cut_entry : (*cut_db->get_cut_map()))
      {
        if(cut_entry.first == 0 ){

          vt::Cut *cut = cut_db->start_reading_cut(cut_entry.first);

          if(!cut->is_drawn())
          {
              cut_db->stop_reading_cut(cut_entry.first);
              continue;
          }
          update_index_texture(ctx, cut_id, vt::Cut::get_dataset_id(cut_entry.first), ctx_id, cut->get_front()->get_index());

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
                 //throw std::runtime_error("updated mem slot inconsistency");
            } else {
              update_physical_texture_blockwise(ctx, ctx_id, mem_slot_updated->pointer, mem_slot_updated->position);
            }

          }
          cut_db->stop_reading_cut(cut_entry.first);
          
        }
       /* else
        {
            cut_db->stop_reading_cut(cut_entry.first);
        }*/
      }
        cut_db->stop_reading();
        render_context->sync();
  }

  void VirtualTexturingRenderer::update_index_texture(gua::RenderContext const& ctx,uint64_t cut_id, uint32_t dataset_id, uint16_t context_id, const uint8_t *buf_cpu) {
    //std::cout << "hello :)";
    auto vector_of_vt_ptr = TextureDatabase::instance()->get_virtual_textures();
    auto phy_tex = TextureDatabase::instance()->lookup("gua_physical_texture_2d");
    //ctx.physical_texture

    for( auto const& vt_ptr : vector_of_vt_ptr ) {
      if(vt_ptr != phy_tex){
      //uint32_t size_index_texture = (*vt::CutDatabase::get_instance().get_cut_map())[cut_id]->get_size_index_texture();
      uint32_t size_index_texture = (uint32_t)vt::QuadTree::get_tiles_per_row((*vt::CutDatabase::get_instance().get_cut_map())[cut_id]->get_atlas()->getDepth() - 1);
      
      scm::math::vec3ui origin = scm::math::vec3ui(0, 0, 0);
      scm::math::vec3ui _index_texture_dimension = scm::math::vec3ui(size_index_texture, size_index_texture, 1);

      vt_ptr->update_sub_data(ctx, scm::gl::texture_region(origin, _index_texture_dimension), 0, scm::gl::FORMAT_RGBA_8UI, buf_cpu);
      }
      else
      {
        std::cout << "Phy tex exists";
      }
    }

  }

  void VirtualTexturingRenderer::update_physical_texture_blockwise(gua::RenderContext const& ctx, uint16_t context_id, const uint8_t *buf_texel, size_t slot_position) {
    auto phy_tex_ptr = ctx.physical_texture;
    auto vector_of_vt_ptr = TextureDatabase::instance()->get_virtual_textures();

    if(phy_tex_ptr == nullptr) {
      //std::cout << "physical_context is nullptr\n";
    } 
    else 
    {
      size_t slots_per_texture = vt::VTConfig::get_instance().get_phys_tex_tile_width() * vt::VTConfig::get_instance().get_phys_tex_tile_width();
      size_t layer = slot_position / slots_per_texture;
      size_t rel_slot_position = slot_position - layer * slots_per_texture;
      size_t x_tile = rel_slot_position % vt::VTConfig::get_instance().get_phys_tex_tile_width();
      size_t y_tile = rel_slot_position / vt::VTConfig::get_instance().get_phys_tex_tile_width();

      scm::math::vec3ui origin = scm::math::vec3ui((uint32_t)x_tile * vt::VTConfig::get_instance().get_size_tile(), (uint32_t)y_tile * vt::VTConfig::get_instance().get_size_tile(), (uint32_t)layer);
      scm::math::vec3ui dimensions = scm::math::vec3ui(vt::VTConfig::get_instance().get_size_tile(), vt::VTConfig::get_instance().get_size_tile(), 1);


      std::cout << "yeah!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";

      std::cout << "origin: " << origin << ", " <<"dimensions: " << dimensions << "\n";

      ctx.render_context->update_sub_texture(phy_tex_ptr, scm::gl::texture_region(origin, dimensions), 0, PhysicalTexture2D::get_tex_format(), buf_texel);
      //phy_tex_ptr->update_sub_texture(ctx, scm::gl::texture_region(origin, dimensions), 0, PhysicalTexture2D::get_tex_format(), buf_texel);
      std::cout << "Done updating the physical texture\n";
      //update_sub_data(ctx, scm::gl::texture_region(origin, dimensions), 0, scm::gl::FORMAT_RGBA_8UI, (void*)(&red_buffer[0]) );



    }

  }


  ///////////////////////////////////////////////////////////////////////////////
  void VirtualTexturingRenderer::render(gua::Pipeline& pipe, PipelinePassDescription const& desc) {

    RenderContext const& ctx(pipe.get_context());

    auto scm_device = ctx.render_device;
    auto scm_context = ctx.render_context;
    //per frame
    //_create_gpu_resources(ctx,cut_id);
    //_cut_update = &(new vt::CutUpdate());
    ///////////////////////////////////////////////////////////////////////////
    //  retrieve current view state
    ///////////////////////////////////////////////////////////////////////////
    auto& scene = *pipe.current_viewstate().scene;
    auto const& camera = pipe.current_viewstate().camera;
    auto const& frustum = pipe.current_viewstate().frustum;
    auto& target = *pipe.current_viewstate().target;

    std::string cpu_query_name_plod_total = "CPU: Camera uuid: " + std::to_string(pipe.current_viewstate().viewpoint_uuid) + " / LodPass";
    pipe.begin_cpu_query(cpu_query_name_plod_total);
    
    //apply cutUpdate with dummy ids = all zero
    apply_cut_update(ctx,0,0);

    auto phys_width = vt::VTConfig::get_instance().get_phys_tex_tile_width();
    auto phys_layers = vt::VTConfig::get_instance().get_phys_tex_layers();
    std::vector<uint32_t> feedback_buffer(phys_width * phys_width * phys_layers, UINT32_MAX);

    vt::CutUpdate::get_instance().feedback(&feedback_buffer[0]);


    //retrieve virtual textures to update them


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
