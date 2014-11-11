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
#include <gua/renderer/TriMeshRenderer.hpp>

#include <gua/renderer/ShaderProgram.hpp>
#include <gua/renderer/Pipeline.hpp>
#include <gua/renderer/TriMeshRessource.hpp>
#include <gua/databases/Resources.hpp>
#include <gua/databases/GeometryDatabase.hpp>
#include <gua/databases/MaterialShaderDatabase.hpp>
#include <gua/node/TriMeshNode.hpp>

#include <math.h>

#define MATERIAL_BUFFER_SIZE 2048*128
#define USE_UBO 0 // also set in MaterialShader.cpp

namespace gua {

////////////////////////////////////////////////////////////////////////////////

TriMeshRenderer::TriMeshRenderer() :
  vertex_shader_(
   Resources::lookup_shader(Resources::shaders_tri_mesh_shader_vert)
 ),
 fragment_shader_(
   Resources::lookup_shader(Resources::shaders_tri_mesh_shader_frag)
 ),
 material_uniform_storage_buffer_(nullptr) {}

////////////////////////////////////////////////////////////////////////////////

void TriMeshRenderer::draw(std::unordered_map<std::string, std::vector<node::GeometryNode*>> const& sorted_objects,
                           Pipeline* pipe) const {

  auto const& ctx(pipe->get_context());

  #if USE_UBO
  // initialize storage buffer
  if (!material_uniform_storage_buffer_) {
    material_uniform_storage_buffer_ = ctx.render_device->create_buffer(
      scm::gl::BIND_UNIFORM_BUFFER, scm::gl::USAGE_STREAM_DRAW, MATERIAL_BUFFER_SIZE);
  }

  // loop through all materials ------------------------------------------------
  for (auto const& object_list : sorted_objects) {

    auto const& material = MaterialShaderDatabase::instance()->lookup(object_list.first);
    if (material) {
      // get shader for this material
      auto tri_mesh_node(reinterpret_cast<node::TriMeshNode*>(object_list.second[0]));
      auto const& shader(material->get_shader(ctx, *tri_mesh_node->get_geometry(), vertex_shader_, fragment_shader_));

      auto max_object_count(material->max_object_count());

      unsigned total_object_count(object_list.second.size());
      unsigned rebind_num(ceil(total_object_count * 1.f / max_object_count));
      int view_id(pipe->get_camera().config.get_view_id());

      for (unsigned current_bind(0); current_bind < rebind_num; ++current_bind) {

        char* buffer = reinterpret_cast<char*>(ctx.render_context->map_buffer(material_uniform_storage_buffer_, scm::gl::ACCESS_WRITE_INVALIDATE_BUFFER));

        unsigned object_count(current_bind < rebind_num - 1 ?
                              max_object_count              :
                              total_object_count - (rebind_num - 1) * max_object_count);

        unsigned current_pos(0);

        // gather all uniform of all nodes for this material ---------------------
        for (int i(0); i < object_count; ++i) {

          int current_object(i + current_bind * max_object_count);
          auto const& node(reinterpret_cast<node::TriMeshNode*>(object_list.second[current_object]));

          UniformValue model_mat(node->get_cached_world_transform());
          UniformValue normal_mat(scm::math::transpose(scm::math::inverse(node->get_cached_world_transform())));

          model_mat.write_bytes(ctx, buffer + current_pos);
          current_pos += model_mat.get_byte_size();
          normal_mat.write_bytes(ctx, buffer + current_pos);
          current_pos += normal_mat.get_byte_size();

          for (auto const& overwrite : node->get_material().get_uniforms()) {

            auto byte_size(overwrite.second.get().get_byte_size());
            auto bytes_left(sizeof(math::vec4) - (current_pos % sizeof(math::vec4)));

            if (bytes_left < byte_size)
              current_pos += bytes_left;

            overwrite.second.get(view_id).write_bytes(ctx, buffer + current_pos);
            current_pos += byte_size;
          }

          auto mod_pos(current_pos % sizeof(math::vec4));
          if (mod_pos != 0) {
            auto bytes_left(sizeof(math::vec4) - mod_pos);
            current_pos += bytes_left;
          }
        }

        ctx.render_context->unmap_buffer(material_uniform_storage_buffer_);

        shader->use(ctx);
        ctx.render_context->apply();

        ctx.render_context->bind_uniform_buffer(material_uniform_storage_buffer_, 1);
        // draw all objects ------------------------------------------------------
        for (int i(0); i < object_count; ++i) {

          int current_object(i + current_bind * max_object_count);
          auto const& node(reinterpret_cast<node::TriMeshNode*>(object_list.second[current_object]));

          if (node->get_geometry()) {
            shader->set_uniform(ctx, i, "gua_draw_index");
            ctx.render_context->apply_program();
            node->get_geometry()->draw(ctx);
          }
        }
      }

    } else {
      Logger::LOG_WARNING << "TriMeshRenderer::draw(): Cannot find material: " << object_list.first << std::endl;
    }
  }

  #else

  int view_id(pipe->get_camera().config.get_view_id());


  // loop through all materials ------------------------------------------------
  for (auto const& object_list : sorted_objects) {

    auto const& material = MaterialShaderDatabase::instance()->lookup(object_list.first);
    if (material) {
      // get shader for this material
      auto tri_mesh_node(reinterpret_cast<node::TriMeshNode*>(object_list.second[0]));
      auto const& shader(material->get_shader(ctx, *tri_mesh_node->get_geometry(), vertex_shader_, fragment_shader_));

      shader->use(ctx);
      ctx.render_context->apply();

      for (auto const& n: object_list.second) {

        auto const& node(reinterpret_cast<node::TriMeshNode*>(n));

        if (node->get_geometry()) {
          UniformValue model_mat(node->get_cached_world_transform());
          UniformValue normal_mat(scm::math::transpose(scm::math::inverse(node->get_cached_world_transform())));

          shader->apply_uniform(ctx, "gua_model_matrix", model_mat);
          shader->apply_uniform(ctx, "gua_normal_matrix", normal_mat);

          for (auto const& overwrite : node->get_material().get_uniforms()) {
            shader->apply_uniform(ctx, overwrite.first, overwrite.second.get(view_id));
          }

          ctx.render_context->apply_program();

          node->get_geometry()->draw(ctx);
        }
      }

    } else {
      Logger::LOG_WARNING << "TriMeshRenderer::draw(): Cannot find material: " << object_list.first << std::endl;
    }
  }

  #endif
}

////////////////////////////////////////////////////////////////////////////////

}
