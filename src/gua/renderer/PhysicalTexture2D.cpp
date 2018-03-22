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
    std::cout << "phy tex wohoo";
    auto render_device = ctx.render_device;
    //auto render_context = ctx.render_context;


    _physical_texture_dimension = scm::math::vec2ui( vt::VTConfig::get_instance().get_phys_tex_px_width(),  vt::VTConfig::get_instance().get_phys_tex_px_width());



    std::cout << "Phys texture dimensions: " << _physical_texture_dimension[0] << " " << _physical_texture_dimension[1] << "\n";
    ctx.physical_texture = render_device->create_texture_2d(_physical_texture_dimension, scm::gl::FORMAT_RGBA_8UI, 1, vt::VTConfig::get_instance().get_phys_tex_layers() + 1) ;

    std::cout << "done creating physical texture\n";
    
  }

void PhysicalTexture2D::upload_to(RenderContext const& context) const {
  RenderContext::Texture ctex{};
  std::cout << "hello from upload_to" << std::endl;

  {
    /*
    ctex.texture = context.render_device->create_texture_2d(
        math::vec2ui(width_, height_), color_format_, 1);
    */
    //initialize_index_texture(context, 0);

    if(!context.physical_texture) {
      //initialize_physical_texture(context);
    }
  }
}


  void PhysicalTexture2D::update(RenderContext const& context) const {

    std::cout << "Performing cut update for VT2D!\n";

    /*std::vector<unsigned char> rand_buffer(_index_texture_dimension[0] * _index_texture_dimension[1] * 4);

    for(unsigned int y_idx = 0; y_idx < _index_texture_dimension[1]; ++y_idx) {
      for(unsigned int x_idx = 0; x_idx < _index_texture_dimension[0]; ++x_idx) {
        unsigned int pixel_offset = ( y_idx * _index_texture_dimension[0] + x_idx ) * 4;

        rand_buffer[ pixel_offset + 0 ] = std::rand() % 255;
        rand_buffer[ pixel_offset + 1 ] = std::rand() % 255;
        rand_buffer[ pixel_offset + 2 ] = std::rand() % 255;
        rand_buffer[ pixel_offset + 3 ] = std::rand() % 255;

      }
    }

    scm::math::vec3ui origin = scm::math::vec3ui(0, 0, 0);
    scm::math::vec3ui dimensions = scm::math::vec3ui(_index_texture_dimension[0], _index_texture_dimension[1], 1); 
    update_sub_data(context, scm::gl::texture_region(origin, dimensions), 0, scm::gl::FORMAT_RGBA_8UI, (void*)(&rand_buffer[0]) );

    //delete cut_update;
    
*/
  };



  //initialize(*context);
/*
  if (ctex.texture) {
    ctex.sampler_state =
        context.render_device->create_sampler_state(state_descripton_);

    context.textures[uuid_] = ctex;
    context.render_context->make_resident(ctex.texture, ctex.sampler_state);
  }
*/


}
