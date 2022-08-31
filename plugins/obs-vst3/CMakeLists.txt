project(obs-vst3)

add_library(obs-vst3 MODULE)


target_include_directories(
  obs-vst3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                  ${CMAKE_CURRENT_BINARY_DIR})

target_sources(
  obs-vst3
  PRIVATE obs-vst3.cpp)

target_link_libraries(obs-vst3 PRIVATE OBS::libobs)

set_target_properties(obs-vst3 PROPERTIES FOLDER "plugins" PREFIX "")

setup_plugin_target(obs-vst3)
