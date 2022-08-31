project(obs-vst3)

add_library(obs-vst3 MODULE)

target_sources(
  obs-vst3
  PRIVATE obs-vst3.cpp)

set_target_properties(obs-vst3 PROPERTIES FOLDER "plugins" PREFIX "")

setup_plugin_target(obs-vst3)
