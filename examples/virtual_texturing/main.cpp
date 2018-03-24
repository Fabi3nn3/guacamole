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

#include <functional>

#include <gua/guacamole.hpp>
#include <gua/renderer/TriMeshLoader.hpp>
#include <gua/renderer/ToneMappingPass.hpp>
#include <gua/renderer/SSAAPass.hpp>
#include <gua/renderer/BBoxPass.hpp>
#include <gua/renderer/DebugViewPass.hpp>
#include <gua/renderer/VirtualTexturingPass.hpp>
#include <scm/gl_util/manipulators/trackball_manipulator.h>

#include <gua/renderer/VTTexture2D.hpp>

#include <lamure/vt/VTConfig.h>
#include <lamure/vt/ren/CutUpdate.h>
#include <lamure/vt/ren/CutDatabase.h>
#include <gua/renderer/VirtualTexturingRenderer.hpp>
#include <gua/renderer/VTTexture2D.hpp>

#include <gua/utils/Trackball.hpp>


//#include <lamure/vt/ren/VTController.h>
void mouse_button(gua::utils::Trackball& trackball,
                  int mousebutton,
                  int action,
                  int mods) {
  gua::utils::Trackball::button_type button;
  gua::utils::Trackball::state_type state;

  switch (mousebutton) {
    case 0:
      button = gua::utils::Trackball::left;
      break;
    case 2:
      button = gua::utils::Trackball::middle;
      break;
    case 1:
      button = gua::utils::Trackball::right;
      break;
  };

  switch (action) {
    case 0:
      state = gua::utils::Trackball::released;
      break;
    case 1:
      state = gua::utils::Trackball::pressed;
      break;
  };

  trackball.mouse(button, state, trackball.posx(), trackball.posy());
}

int main(int argc, char** argv) {

  gua::init(argc, argv);

  //setup scenegraph
  gua::SceneGraph graph("main_scenegraph");
  auto scene_transform = graph.add_node<gua::node::TransformNode>("/", "transform");

  auto screen = graph.add_node<gua::node::ScreenNode>("/", "screen");
  screen->data.set_size(gua::math::vec2(1.92f, 1.08f));
  screen->translate(0, 0, 1.0);

  //add mouse interaction
  gua::utils::Trackball trackball(0.01, 0.002, 0.2);
  /*scm::gl::trackball_manipulator trackball;
  trackball.transform_matrix()*scm::math::make_translation(0.01f, 0.002f, 0.2f);
  trackball.dolly(0.2f);*/

  float dolly_sens = 1.5f;
  gua::math::vec2 trackball_init_pos(0.f);
  gua::math::vec2 last_mouse_pos(0.f);
  int button_state = -1;



  auto load_mat = [](std::string const& file){
    auto desc(std::make_shared<gua::MaterialShaderDescription>());
    desc->load_from_file(file);
    auto shader(std::make_shared<gua::MaterialShader>(file, desc));
    gua::MaterialShaderDatabase::instance()->add(shader);
    return shader->make_new_material();
  };

  gua::TriMeshLoader loader;

  //  std::string file_config = std::string("/mnt/terabytes_of_textures/FINAL_DEMO_DATA/configuration_template.ini");
  std::string file_config = std::string("/mnt/terabytes_of_textures/FINAL_DEMO_DATA/configuration_template_gua_test.ini");
  std::string file_atlas = std::string("/mnt/terabytes_of_textures/FINAL_DEMO_DATA/earth_stitch_86400x43200_256x256_p1_rgb_packed.atlas");

  vt::VTConfig::CONFIG_PATH = file_config;
  std::cout << "CONFIG PATH IS: " << vt::VTConfig::CONFIG_PATH << "\n";

  //GPU Infos wrong - why? answer: don't use plain gl in scm/gua! -> look for wrappers
   //GLint max_tex_layers;
   //glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_tex_layers);

   //GLint max_tex_px_width_gl;
   //glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_px_width_gl);

  //std::cout << "Max PhyTex Layers: " << max_tex_layers << " Max px width: " << max_tex_px_width_gl << "\n";


  vt::VTConfig::get_instance().define_size_physical_texture(50, 4096);

  uint32_t data_id = vt::CutDatabase::get_instance().register_dataset(file_atlas);
  uint16_t view_id = vt::CutDatabase::get_instance().register_view();
  uint16_t primary_context_id = vt::CutDatabase::get_instance().register_context();

  uint64_t cut_id = vt::CutDatabase::get_instance().register_cut(data_id, view_id, primary_context_id);

    auto *cut_update = &vt::CutUpdate::get_instance();
    cut_update->start();

    auto phys_width = vt::VTConfig::get_instance().get_phys_tex_tile_width();
    auto phys_layers = vt::VTConfig::get_instance().get_phys_tex_layers();
    //uint64_t color_cut_id = (((uint64_t)color_data_id) << 32) | ((uint64_t)view_id << 16) | ((uint64_t)context_id);
    uint64_t color_cut_id = 0;
    //uint32_t max_depth_level_color = (*vt::CutDatabase::get_instance().get_cut_map())[color_cut_id]->get_atlas()->getDepth() - 1;
    scm::math::vec2ui phy_tex_dim = scm::math::vec2ui( vt::VTConfig::get_instance().get_phys_tex_px_width(),  vt::VTConfig::get_instance().get_phys_tex_px_width());
    uint16_t tile_size = vt::VTConfig::get_instance().get_size_tile();
    uint16_t tile_padding = vt::VTConfig::get_instance().get_size_padding();
    uint32_t size_index_texture = (uint32_t)vt::QuadTree::get_tiles_per_row((*vt::CutDatabase::get_instance().get_cut_map())[color_cut_id]->get_atlas()->getDepth() - 1);
    scm::math::vec2ui idx_tex_dim = scm::math::vec2ui(size_index_texture, size_index_texture);
    int max_depth_level_color = 9;
    std::cout << "idx_tex_dim: "<<idx_tex_dim;

    auto vt_material(load_mat("data/materials/VirtualTexturing.gmd"));

    //happens in set_uniform too :)
    //gua::TextureDatabase::instance()->load(file_atlas);

    vt_material->set_uniform("ColorMap", std::string(file_atlas));
    std::string phy_tex_name = "gua_physical_texture_2d";
    vt_material->set_uniform("PhysicalTexture2D", std::string(phy_tex_name));
    vt_material->set_uniform("max_level_color", max_depth_level_color);
    vt_material->set_uniform("tile_size", scm::math::vec2ui(tile_size, tile_size) );
    vt_material->set_uniform("tile_padding", scm::math::vec2ui(tile_padding, tile_padding) );
    vt_material->set_uniform("physical_texture_dim",phy_tex_dim);
    vt_material->set_uniform("index_dim_color",idx_tex_dim);


    auto transform = graph.add_node<gua::node::TransformNode>("/", "transform");
    auto quad(loader.create_geometry_from_file(
        "quad", "data/objects/quad.obj",
        vt_material,
        gua::TriMeshLoader::NORMALIZE_POSITION |
        gua::TriMeshLoader::NORMALIZE_SCALE) );

    graph.add_node("/transform", quad);


   quad->scale(1.5f);
  //setup rendering pipeline and window
  auto resolution = gua::math::vec2ui(1920, 1080);

  auto camera = graph.add_node<gua::node::CameraNode>("/screen", "cam");
  camera->translate(0, 0, 2.f);
  camera->config.set_resolution(resolution);
  
  //use close near plane to allow inspection of details
  camera->config.set_near_clip(0.01f);
  camera->config.set_far_clip(20.0f);
  camera->config.set_screen_path("/screen");
  camera->config.set_scene_graph_name("main_scenegraph");
  camera->config.set_output_window_name("main_window");
  camera->config.set_enable_stereo(false);

  auto pipe = std::make_shared<gua::PipelineDescription>();
  pipe->add_pass(std::make_shared<gua::TriMeshPassDescription>());
  pipe->add_pass(std::make_shared<gua::VirtualTexturingPassDescription>());
  pipe->add_pass(std::make_shared<gua::LightVisibilityPassDescription>());
  pipe->add_pass(std::make_shared<gua::ResolvePassDescription>());

  
  camera->set_pipeline_description(pipe);

  pipe->get_resolve_pass()->tone_mapping_exposure(5.f);

  pipe->get_resolve_pass()->background_mode(gua::ResolvePassDescription::BackgroundMode::SKYMAP_TEXTURE);
  //pipe->get_resolve_pass()->background_texture(file_atlas);

  //init window and window behaviour
  auto window = std::make_shared<gua::GlfwWindow>();
  gua::WindowDatabase::instance()->add("main_window", window);
  window->config.set_enable_vsync(false);
  window->config.set_size(resolution);
  window->config.set_resolution(resolution);
  window->config.set_stereo_mode(gua::StereoMode::MONO);

  window->on_resize.connect([&](gua::math::vec2ui const& new_size) {
    window->config.set_resolution(new_size);
    camera->config.set_resolution(new_size);
    screen->data.set_size(gua::math::vec2(0.001 * new_size.x, 0.001 * new_size.y));
  });



  window->on_button_press.connect(
      std::bind(mouse_button, std::ref(trackball), std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
  window->open();


    auto *cutupdate = &vt::CutUpdate::get_instance();
    cutupdate->start();
    //_cut_update_started = true;

  gua::Renderer renderer;

  //application loop
  gua::events::MainLoop loop;
  gua::events::Ticker ticker(loop, 1.0 / 500.0);

  ticker.on_tick.connect([&]() {
  gua::math::mat4 modelmatrix =
        scm::math::make_translation(gua::math::float_t(trackball.shiftx()),
                                    gua::math::float_t(trackball.shifty()),
                                    gua::math::float_t(trackball.distance())) *
        gua::math::mat4(trackball.rotation());

    scene_transform->set_transform(modelmatrix);

    window->process_events();
    if (window->should_close()) {
      renderer.stop();
      window->close();
      loop.stop();
    }
    else {
      renderer.queue_draw({ &graph });
    }
  });

  loop.start();

  return 0;
}
