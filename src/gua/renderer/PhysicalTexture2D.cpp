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
#include <gua/renderer/PhysicalTexture2D.hpp>

// guacamole headers
#include <FreeImagePlus.h>
#include <gua/platform.hpp>
#include <gua/utils/Logger.hpp>
#include <gua/math/math.hpp>

#include <boost/shared_ptr.hpp>
#include <scm/gl_core/data_formats.h>
#include <scm/gl_core/log.h>
#include <scm/gl_core/render_device.h>
#include <scm/gl_core/texture_objects.h>

#include <scm/gl_util/data/imaging/texture_loader.h>
#include <scm/gl_util/data/imaging/texture_image_data.h>
#include <lamure/vt/VTConfig.h>
#include <lamure/vt/ren/CutUpdate.h>
#include <lamure/vt/ren/CutDatabase.h>
#include <lamure/vt/QuadTree.h>

#include <iostream>
#include <condition_variable>

namespace gua {

PhysicalTexture2D::PhysicalTexture2D(std::string const& file,
                     scm::gl::sampler_state_desc const& state_descripton)
    : Texture(file, 0, state_descripton),
      width_(0),
      height_(0) {}  

  void PhysicalTexture2D::initialize_physical_texture(RenderContext const& ctx) const{

    RenderContext::Texture ctex;
    auto scm_device = ctx.render_device;
    auto scm_context = ctx.render_context;
    
    _physical_texture_dimension = scm::math::vec2ui( vt::VTConfig::get_instance().get_phys_tex_px_width(),  vt::VTConfig::get_instance().get_phys_tex_px_width());
    std::cout << "Phys texture dimensions: " << _physical_texture_dimension[0] << " " << _physical_texture_dimension[1] << "\n";

    ctex.texture = ctx.render_device->create_texture_2d(_physical_texture_dimension, get_tex_format(), 1, vt::VTConfig::get_instance().get_phys_tex_layers() +1) ;

    ctx.size_feedback = vt::VTConfig::get_instance().get_phys_tex_tile_width() * vt::VTConfig::get_instance().get_phys_tex_tile_width() * vt::VTConfig::get_instance().get_phys_tex_layers();
    ctx.feedback_storage = scm_device->create_buffer(scm::gl::BIND_STORAGE_BUFFER, scm::gl::USAGE_DYNAMIC_READ, ctx.size_feedback * size_of_format(scm::gl::FORMAT_R_32UI));
    ctx.feedback_cpu_buffer = new uint32_t[ctx.size_feedback];

    scm_context->bind_storage_buffer(ctx.feedback_storage, 20);

    if (ctex.texture)
    {
      auto filter_nearest_descriptor = scm::gl::sampler_state_desc(scm::gl::FILTER_MIN_MAG_NEAREST,
                                                                   scm::gl::WRAP_REPEAT,
                                                                   scm::gl::WRAP_REPEAT);

      ctex.sampler_state = ctx.render_device->create_sampler_state(state_descripton_);

      ctx.textures[uuid_] = ctex;
      ctx.physical_texture = ctex.texture;

      ctx.render_context->make_resident(ctex.texture, ctex.sampler_state);
    } 
    else 
    {
      std::cout << "FAILED TO CREATE PHYSICAL TEXTURE\n";
    }
    
  }

void PhysicalTexture2D::upload_to(RenderContext const& context) const {
  if(!context.physical_texture)
  { 
    initialize_physical_texture(context); 
  }
}

scm::gl::data_format PhysicalTexture2D::get_tex_format()        
{                                                        
    switch(vt::VTConfig::get_instance().get_format_texture())
    {                                                    
    case vt::VTConfig::R8:                                   
        return scm::gl::FORMAT_R_8;                      
    case vt::VTConfig::RGB8:                                 
        return scm::gl::FORMAT_RGB_8;                    
    case vt::VTConfig::RGBA8:                                
    default:
      return scm::gl::FORMAT_RGBA_8;                   
    }                                                    
}                                                        

}
