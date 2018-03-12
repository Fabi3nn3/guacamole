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

//#include <lamure/vt/ren/VTController.h>

int main(int argc, char** argv) {

  gua::init(argc, argv);

  //setup scenegraph
  gua::SceneGraph graph("main_scenegraph");
  auto scene_transform = graph.add_node<gua::node::TransformNode>("/", "transform");

  auto screen = graph.add_node<gua::node::ScreenNode>("/", "screen");
  screen->data.set_size(gua::math::vec2(1.92f, 1.08f));
  screen->translate(0, 0, 1.0);

  //add mouse interaction
  scm::gl::trackball_manipulator trackball;
  trackball.transform_matrix()*scm::math::make_translation(0.01f, 0.002f, 0.2f);
  trackball.dolly(0.2f);
  float dolly_sens = 1.5f;
  gua::math::vec2 trackball_init_pos(0.f);
  gua::math::vec2 last_mouse_pos(0.f);
  int button_state = -1;




    std::string file_config = std::string("/mnt/terabytes_of_textures/FINAL_DEMO_DATA/configuration_template.ini");
    std::string file_atlas = std::string("/mnt/terabytes_of_textures/FINAL_DEMO_DATA/earth_stitch_86400x43200_256x256_p1_rgb_packed.atlas");
    vt::VTConfig::CONFIG_PATH = file_config;

    //calls read_config when creating instance

     GLint max_tex_layers;
     glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_tex_layers);

     GLint max_tex_px_width_gl;
     glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_px_width_gl);
 
    std::cout << "Defined Physical Texture with: " << max_tex_layers << " Layers and " << max_tex_px_width_gl<< " max_px_width_gl" << std::endl;

    vt::VTConfig::get_instance().define_size_physical_texture((int32_t) max_tex_layers,(int32_t) max_tex_px_width_gl);

    uint32_t data_id = vt::CutDatabase::get_instance().register_dataset(file_atlas);
    uint16_t view_id = vt::CutDatabase::get_instance().register_view();
    uint16_t primary_context_id = vt::CutDatabase::get_instance().register_context();

    uint64_t cut_id = vt::CutDatabase::get_instance().register_cut(data_id, view_id, primary_context_id);

    auto *cut_update = new vt::CutUpdate();
    cut_update->start();

    uint32_t phy_tex_dim_width = vt::VTConfig::get_instance().get_phys_tex_px_width();
    std::cout << "Phy Tex Dim: "<< phy_tex_dim_width << std::endl;
    //create phy tex -> .atlas possible


    //own function
    vt::VTConfig::FORMAT_TEXTURE phy_tex_format = vt::VTConfig::get_instance().get_format_texture();
    uint32_t physical_texture_width = vt::VTConfig::get_instance().get_phys_tex_px_width();
    uint16_t physical_texture_layers = vt::VTConfig::get_instance().get_phys_tex_layers();
    gua::math::vec2ui physical_texture_dim = gua::math::vec2ui(physical_texture_width);
    std::cout <<"phy tex dim: "<< physical_texture_dim << endl;

    
    gua::VTTexture2D physical_texture(physical_texture_dim,physical_texture_layers,phy_tex_format);


    //gua::VTTexture2D physical_texture(phy_tex_dim_width, phy_tex_dim_width,vt::VTConfig::get_instance().get_format_texture(),vt::VTConfig::get_instance().get_phys_tex_layers()+1,);

  //BEFORE ANY VT IS LOADED: TELL THE CONTROLLER HOW MUCH MEMORY SHOULD BE ALLOCATED FOR THE BUDGET!

  /*lamure::vt::VTController* vt_controller_p = (lamure::vt::VTController*) lamure::vt::VTController::get_instance();
  
  vt_controller_p->cpu_budget_in_mb = 4096;
  vt_controller_p->gpu_budget_in_mb = 1024;
  vt_controller_p->cut_update_budget_in_mb = 10;*/


  //LOAD TEXTURE ATLAS
  //gua::TextureDatabase::instance()->load("MYFILE.atlas");




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
  pipe->get_resolve_pass()->background_texture("data/textures/envlightmap.jpg");

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




  window->open();

  gua::Renderer renderer;

  gua::VirtualTexturingRenderer vtrenderer;
  vtrenderer.apply_cutupdate(primary_context_id);
  //application loop
  gua::events::MainLoop loop;
  gua::events::Ticker ticker(loop, 1.0 / 500.0);

  ticker.on_tick.connect([&]() {

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
