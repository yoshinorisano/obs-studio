project(obs-vst3)

# TODO
add_link_options("/IGNORE:4075")

add_library(obs-vst3 MODULE)

find_qt(COMPONENTS Widgets)

set_target_properties(
  obs-vst3
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

target_include_directories(
  obs-vst3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                  ${CMAKE_CURRENT_BINARY_DIR}
                  "C:\\VST_SDK\\vst3sdk") # TODO

target_sources(
  obs-vst3
  PRIVATE obs-vst3.cpp VST3Plugin.cpp EditorWidget.cpp
          VST3Plugin.h EditorWidget.h)

target_link_directories(obs-vst3 PRIVATE "C:\\VST_SDK\\vst3sdk\\out\\build\\x64-Debug\\lib") # TODO

target_link_libraries(obs-vst3 PRIVATE OBS::libobs Qt::Widgets "base.lib" "pluginterfaces.lib" "sdk.lib" "sdk_common.lib" "sdk_hosting.lib" "vstgui_standalone.lib")

set_target_properties(obs-vst3 PROPERTIES FOLDER "plugins" PREFIX "")

setup_plugin_target(obs-vst3)
