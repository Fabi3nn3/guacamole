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

#ifndef GUA_VTTEXTURE2D_HPP
#define GUA_VTTEXTURE2D_HPP

// guacamole headers
#include <gua/platform.hpp>
#include <gua/renderer/RenderContext.hpp>
#include <gua/renderer/Texture.hpp>
#include <gua/math/math.hpp>
#include <gua/utils/Logger.hpp>
#include <lamure/vt/VTConfig.h>
#include <scm/gl_util/data/imaging/texture_image_data.h>

// external headers
#include <string>
#include <vector>

#include <mutex>
#include <thread>
#include <memory>

namespace gua {

class GUA_DLL VTTexture2D : public Texture {
 public:
  
  VTTexture2D(std::string const& file,
              scm::gl::sampler_state_desc const& state_descripton =
              scm::gl::sampler_state_desc(scm::gl::FILTER_ANISOTROPIC,
                                          scm::gl::WRAP_REPEAT,
                                          scm::gl::WRAP_REPEAT));

  unsigned width() const override { return width_; }
  unsigned height() const override { return height_; }

  void upload_to(RenderContext const& context) const override;
  void initialize_index_texture(RenderContext const& ctx, uint64_t cut_id) const;

  void update(RenderContext const& context) const override;

  ///@}

 protected:
  scm::gl::texture_image_data_ptr image_ = nullptr;
  unsigned width_;
  unsigned height_;
  unsigned layers_;

 private:
  std::string _file_config;
  std::string _file_atlas;  
  scm::shared_ptr<scm::gl::render_device> _device;
  mutable scm::math::vec2ui                                                         _index_texture_dimension;
  mutable scm::math::vec2ui                                                         _physical_texture_dimension;
  scm::gl::sampler_state_ptr                                                        _filter_nearest;
  scm::gl::sampler_state_ptr                                                        _filter_linear;
};  

}
#endif  // GUA_VTTEXTURE2D_HPP
