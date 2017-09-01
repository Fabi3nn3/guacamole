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

#ifndef GUA_LINE_STRIP_RESOURCE_HPP
#define GUA_LINE_STRIP_RESOURCE_HPP

// guacamole headers
#include <gua/platform.hpp>
#include <gua/renderer/GeometryResource.hpp>
#include <gua/utils/LineStrip.hpp>
#include <gua/utils/KDTree.hpp>

// external headers
#include <scm/gl_core.h>

#include <mutex>
#include <thread>

#include <vector>

namespace gua {

struct RenderContext;

/**
 * Stores geometry data.
 *
 * A line strip can be loaded from an *.lob file and the draw onto multiple
 * contexts.
 * Do not use this class directly, it is just used by the Geometry class to
 * store the individual meshes of a file.
 */
class LineStripResource : public GeometryResource {
 public:

  /**
   * Default constructor.
   *
   * Creates a new and empty Line Strip.
   */
   LineStripResource();

  /**
   * Constructor from an *.lob strip.
   *
   * Initializes the strip from a given *.lob strip.
   *
   * \param mesh             The line strip to load the data from.
   */
   LineStripResource(LineStrip const& line_strip, bool build_kd_tree);

  /**
   * Draws the line strip.
   *
   * Draws the line strip to the given context.
   *
   * \param context          The RenderContext to draw onto.
   */
  void draw(RenderContext& context) const;

  void ray_test(Ray const& ray, int options,
                node::Node* owner, std::set<PickResult>& hits) override;

  inline unsigned int num_occupied_vertex_slots() const { return line_strip_.num_occupied_vertex_slots; }
  inline unsigned int vertex_reservoir_size() const { return line_strip_.vertex_reservoir_size; }
  
  math::vec3 get_vertex(unsigned int i) const;
  std::vector<unsigned int> get_face(unsigned int i) const;

 private:

  void upload_to(RenderContext& context) const;

  KDTree kd_tree_;
  LineStrip line_strip_;
};

}

#endif  // GUA_LINE_STRIP_RESOURCE_HPP
