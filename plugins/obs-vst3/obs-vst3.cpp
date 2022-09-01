#include "VST3Plugin.h"
#include <obs-module.h>

#define OPEN_VST3_SETTINGS "open_vst3_settings"

#define PLUG_IN_NAME "VST 3.x Plug-in"
#define OPEN_VST3_TEXT "Open Plug-in Interface"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs3-vst", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "VST 3.x Plug-in filter";
}

static bool open_editor_button_clicked(obs_properties_t *props,
				       obs_property_t *property, void *data)
{
	VST3Plugin *vst3Plugin = (VST3Plugin *)data;

	// TODO
	vst3Plugin->loadEffectFromPath(std::string("C:\\Program Files\\Common Files\\VST3\\OrilRiver.vst3"));

	QMetaObject::invokeMethod(vst3Plugin, "openEditor");

	return true;
}

static const char *vst3_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return PLUG_IN_NAME;
}

static void *vst3_create(obs_data_t *settings, obs_source_t *filter)
{
	VST3Plugin *vst3Plugin = new VST3Plugin();
	return vst3Plugin;
}

static void vst3_destroy(void *data)
{

}

static void vst3_update(void* data, obs_data_t* settings)
{

}

static struct obs_audio_data *vst3_filter_audio(void *data,
					       struct obs_audio_data *audio)
{
	return NULL;
}

static obs_properties_t *vst3_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();
	obs_properties_add_button(props, OPEN_VST3_SETTINGS, OPEN_VST3_TEXT,
				  open_editor_button_clicked);
	return props;
}

static void vst3_save(void *data, obs_data_t *settings)
{

}

bool obs_module_load(void)
{
	struct obs_source_info vst3_filter = {};
	vst3_filter.id = "vst3_filter";
	vst3_filter.type = OBS_SOURCE_TYPE_FILTER;
	vst3_filter.output_flags = OBS_SOURCE_AUDIO;
	vst3_filter.get_name = vst3_name;
	vst3_filter.create = vst3_create;
	vst3_filter.destroy = vst3_destroy;
	vst3_filter.update = vst3_update;
	vst3_filter.filter_audio = vst3_filter_audio;
	vst3_filter.get_properties = vst3_properties;
	vst3_filter.save = vst3_save;

	obs_register_source(&vst3_filter);
	return true;
}
