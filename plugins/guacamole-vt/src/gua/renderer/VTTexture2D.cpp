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

#include <iostream>

namespace gua {

VTTexture2D::VTTexture2D()
{}

VTTexture2D::VTTexture2D(scm::gl::texture_image_data_ptr image,
                     unsigned mipmap_layers,
                     scm::gl::sampler_state_desc const& state_descripton)
    : Texture(image->format(),
              image->format(),
              mipmap_layers,
              state_descripton),
      image_(image),
      width_(image ? image->mip_level(0).size().x : 0),
      height_(image ? image->mip_level(0).size().y : 0) {
}

VTTexture2D::VTTexture2D(unsigned width,
                     unsigned height,
                     scm::gl::data_format color_format,
                     unsigned mipmap_layers,
                     scm::gl::sampler_state_desc const& state_descripton)
    : Texture(color_format, mipmap_layers, state_descripton),
      image_(nullptr),
      width_(width),
      height_(height) {
}

VTTexture2D::VTTexture2D(std::string const& file,
                     bool generate_mipmaps,
                     scm::gl::sampler_state_desc const& state_descripton)
    : Texture(file, generate_mipmaps, state_descripton),
      image_(load_image_2d(file, mipmap_layers_ > 0)),
      width_(image_ ? image_->mip_level(0).size().x : 0),
      height_(image_ ? image_->mip_level(0).size().y : 0) {
}


VTTexture2D::VTTexture2D(gua::math::vec2ui dim, uint16_t layers, vt::VTConfig::FORMAT_TEXTURE format):
width_(dim.x),
height_(dim.y),
layers_(layers)
{
 //create_texture_2d(dim,format,0,layers);

  /*
  create_texture_2d(const math::vec2ui& in_size,
                                                      const data_format   in_format,
                                                      const unsigned      in_mip_levels = 1,
                                                      const unsigned      in_array_layers = 1,
                                                      const unsigned      in_samples = 1);
  */
}

void VTTexture2D::initialize(RenderContext *context){
    std::cout << "yeah";

    //_filter_nearest = render_device->create_sampler_state(scm::gl::FILTER_MIN_MAG_NEAREST, scm::gl::WRAP_CLAMP_TO_EDGE);
    //_filter_linear = render_device->create_sampler_state(scm::gl::FILTER_MIN_MAG_LINEAR, scm::gl::WRAP_CLAMP_TO_EDGE); 
  }

  void VTTexture2D::initialize_index_texture(RenderContext const& ctx, uint64_t cut_id){

    auto render_device = ctx.render_device;
    auto render_context = ctx.render_context;

    uint32_t size_index_texture = (*vt::CutDatabase::get_instance().get_cut_map())[cut_id]->get_size_index_texture();
    _index_texture_dimension = scm::math::vec2ui(size_index_texture, size_index_texture);
    _index_texture = render_device->create_texture_2d(_index_texture_dimension, scm::gl::FORMAT_RGBA_8UI);

  }

  void VTTexture2D::initialize_physical_texture(RenderContext const& ctx){
    std::cout << "phy tex wohoo";
    auto render_device = ctx.render_device;
    auto render_context = ctx.render_context;

    _physical_texture_dimension = scm::math::vec2ui(vt::VTConfig::get_instance().get_phys_tex_tile_width(), vt::VTConfig::get_instance().get_phys_tex_tile_width());
    _physical_texture = render_device->create_texture_2d(_physical_texture_dimension, scm::gl::FORMAT_RGBA_8UI, 0, vt::VTConfig::get_instance().get_phys_tex_layers() + 1);
    
  }

void VTTexture2D::upload_to(RenderContext const& context) const {
  RenderContext::Texture ctex{};

  if (image_) {
    std::vector<void*> data;
    for (unsigned i = 0; i < image_->mip_level_count(); ++i)
      data.push_back(image_->mip_level(i).data().get());

    ctex.texture = context.render_device->create_texture_2d(
        image_->mip_level(0).size(), image_->format(),
        image_->mip_level_count(), 1, 1, image_->format(), data);
  } else {
    ctex.texture = context.render_device->create_texture_2d(
        math::vec2ui(width_, height_), color_format_, mipmap_layers_);
  }
  //initialize(*context);

  if (ctex.texture) {
    ctex.sampler_state =
        context.render_device->create_sampler_state(state_descripton_);

    context.textures[uuid_] = ctex;
    context.render_context->make_resident(ctex.texture, ctex.sampler_state);
  }
}

scm::gl::texture_image_data_ptr load_image_2d(std::string const& filename,
                                              bool create_mips) {
  scm::scoped_ptr<fipImage> in_image(new fipImage);

  if (!in_image->load(filename.c_str())) {
    scm::gl::glerr() << scm::log::error << "texture_loader::load_image_2d(): "
                     << "unable to open file: " << filename << scm::log::end;
    return scm::gl::texture_image_data_ptr();
  }

  FREE_IMAGE_TYPE image_type = in_image->getImageType();
  math::vec2ui image_size(in_image->getWidth(), in_image->getHeight());
  scm::gl::data_format format = scm::gl::FORMAT_NULL;
  unsigned image_bit_count = in_image->getInfoHeader()->biBitCount;

  switch (image_type) {
    case FIT_BITMAP: {
      unsigned num_components = in_image->getBitsPerPixel() / 8;
      switch (num_components) {
        case 1:
          format = scm::gl::FORMAT_R_8;
          break;
        case 2:
          format = scm::gl::FORMAT_RG_8;
          break;
        case 3:
          format = scm::gl::FORMAT_BGR_8;
          break;
        case 4:
          format = scm::gl::FORMAT_BGRA_8;
          break;
      }
    } break;
    case FIT_INT16:
      format = scm::gl::FORMAT_R_16S;
      break;
    case FIT_UINT16:
      format = scm::gl::FORMAT_R_16;
      break;
    case FIT_RGB16:
      format = scm::gl::FORMAT_RGB_16;
      break;
    case FIT_RGBA16:
      format = scm::gl::FORMAT_RGBA_16;
      break;
    case FIT_INT32:
      break;
    case FIT_UINT32:
      break;
    case FIT_FLOAT:
      format = scm::gl::FORMAT_R_32F;
      break;
    case FIT_RGBF:
      format = scm::gl::FORMAT_RGB_32F;
      break;
    case FIT_RGBAF:
      format = scm::gl::FORMAT_RGBA_32F;
      break;
  }

  if (format == scm::gl::FORMAT_NULL) {
    scm::gl::glerr() << scm::log::error << "texture_loader::load_image_2d(): "
                     << "unsupported color format: " << std::hex
                     << in_image->getImageType() << scm::log::end;
    return scm::gl::texture_image_data_ptr();
  }

  scm::gl::texture_image_data::level_vector mip_vec;

  unsigned num_mip_levels = 1;
  if (create_mips) {
    num_mip_levels = scm::gl::util::max_mip_levels(image_size);
  }

  for (unsigned i = 0; i < num_mip_levels; ++i) {
    scm::size_t cur_data_size = 0;
    math::vec2ui lev_size = scm::gl::util::mip_level_dimensions(image_size, i);

    if (i == 0) {
      lev_size = image_size;
      cur_data_size = image_size.x * image_size.y;
      cur_data_size *= channel_count(format);
      cur_data_size *= size_of_channel(format);
    } else {
      cur_data_size = lev_size.x * lev_size.y;
      cur_data_size *= channel_count(format);
      cur_data_size *= size_of_channel(format);

      if (FALSE == in_image->rescale(lev_size.x, lev_size.y, FILTER_LANCZOS3)) {
        scm::gl::glerr() << scm::log::error
                         << "texture_loader::load_image_2d(): "
                         << "unable to scale image (level: " << i
                         << ", dim: " << lev_size << ")" << scm::log::end;
        return scm::gl::texture_image_data_ptr();
      }

      if (in_image->getWidth() != lev_size.x ||
          in_image->getHeight() != lev_size.y) {
        scm::gl::glerr() << scm::log::error
                         << "texture_loader::load_image_2d(): "
                         << "image dimensions changed after resamling (level: "
                         << i << ", dim: " << lev_size << ", type: " << std::hex
                         << in_image->getImageType() << ")" << scm::log::end;
        return scm::gl::texture_image_data_ptr();
      }
      if (in_image->getInfoHeader()->biBitCount != image_bit_count) {
        scm::gl::glerr() << scm::log::error
                         << "texture_loader::load_image_2d(): "
                         << "image bitcount changed after resamling (level: "
                         << i << ", bit_count: " << image_bit_count
                         << ", img_bit_count: "
                         << in_image->getInfoHeader()->biBitCount << ")"
                         << scm::log::end;
        return scm::gl::texture_image_data_ptr();
      }
      if (image_type != in_image->getImageType()) {
        scm::gl::glerr() << scm::log::error
                         << "texture_loader::load_image_2d(): "
                         << "image type changed after resamling (level: " << i
                         << ", dim: " << lev_size << ", type: " << std::hex
                         << in_image->getImageType() << ")" << scm::log::end;
        return scm::gl::texture_image_data_ptr();
      }
    }

    scm::shared_array<unsigned char> cur_data(new unsigned char[cur_data_size]);

    size_t line_pitch = in_image->getScanWidth();
    for (unsigned l = 0; l < lev_size.y; ++l) {
      size_t ls = static_cast<size_t>(lev_size.x) * size_of_format(format);
      uint8_t* s =
          reinterpret_cast<uint8_t*>(in_image->accessPixels()) + line_pitch * l;
      uint8_t* d = reinterpret_cast<uint8_t*>(cur_data.get()) + ls * l;
      std::memcpy(d, s, ls);
    }

    mip_vec.push_back({lev_size, cur_data});
  }

  return boost::make_shared<scm::gl::texture_image_data>(
      scm::gl::texture_image_data::ORIGIN_LOWER_LEFT, format, mip_vec);
}
}
