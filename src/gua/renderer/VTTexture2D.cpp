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
#include <gua/renderer/VTTexture2D.hpp>

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

VTTexture2D::VTTexture2D(std::string const& file,
                     scm::gl::sampler_state_desc const& state_descripton)
    : Texture(file, 0, state_descripton),
      width_(0),
      height_(0) {

}


  void VTTexture2D::initialize_index_texture(RenderContext const& ctx, uint64_t cut_id) const {
    
    auto render_device = ctx.render_device;
    auto render_context = ctx.render_context;

    uint32_t size_index_texture = (uint32_t)vt::QuadTree::get_tiles_per_row((*vt::CutDatabase::get_instance().get_cut_map())[cut_id]->get_atlas()->getDepth() - 1);
    _index_texture_dimension = scm::math::vec2ui(size_index_texture, size_index_texture);

    RenderContext::Texture ctex;
    ctex.texture = ctx.render_device->create_texture_2d(_index_texture_dimension, scm::gl::FORMAT_RGBA_8UI);

    scm::math::vec3ui origin = scm::math::vec3ui(0, 0, 0);
    scm::math::vec3ui dimensions = scm::math::vec3ui(_index_texture_dimension[0], _index_texture_dimension[1], 1);


    std::vector<unsigned char> red_buffer(_index_texture_dimension[0] * _index_texture_dimension[1] * 4);

    for(unsigned int y_idx = 0; y_idx < _index_texture_dimension[1]; ++y_idx) {
      for(unsigned int x_idx = 0; x_idx < _index_texture_dimension[0]; ++x_idx) {
        unsigned int pixel_offset = ( y_idx * _index_texture_dimension[0] + x_idx ) * 4;

        red_buffer[ pixel_offset + 0 ] = 0;
        red_buffer[ pixel_offset + 1 ] = 0;
        red_buffer[ pixel_offset + 2 ] = 0;
        red_buffer[ pixel_offset + 3 ] = 0;

      }
    }

    if (ctex.texture) {
      auto filter_nearest_descriptor = scm::gl::sampler_state_desc(scm::gl::FILTER_MIN_MAG_NEAREST,
                                                                   scm::gl::WRAP_REPEAT,
                                                                   scm::gl::WRAP_REPEAT);

      ctex.sampler_state = ctx.render_device->create_sampler_state(state_descripton_);

      ctx.textures[uuid_] = ctex;
      ctx.render_context->make_resident(ctex.texture, ctex.sampler_state);

      update_sub_data(ctx, scm::gl::texture_region(origin, dimensions), 0, scm::gl::FORMAT_RGBA_8UI, (void*)(&red_buffer[0]) );
    } else {
      std::cout << "FAILED TO CREATE INDEX TEXTURE\n";
    }

  }

  
void VTTexture2D::upload_to(RenderContext const& context) const {
  RenderContext::Texture ctex{};
  {

    initialize_index_texture(context, 0);

    if(!context.physical_texture) {
  
    }
  }
}


}
