#ifndef OBS_STUDIO_VST3PLUGIN_H
#define OBS_STUDIO_VST3PLUGIN_H

#include <string>
#include <QObject>
#include <Windows.h>

#include "obs.h"

// TODO
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"

class EditorWidget;

class VST3Plugin : public QObject {
	Q_OBJECT

	EditorWidget *editorWidget = nullptr;
	obs_source_t *sourceContext;
	std::string pluginPath;

	void loadEffect();

	std::string sourceName;
	std::string filterName;
	char effectName[64];

	HINSTANCE dllHandle = nullptr;

	Steinberg::Vst::IAudioProcessor *audioProcessor;
	Steinberg::Vst::IEditController *editController;

public:
	VST3Plugin(obs_source_t *sourceContext);
	void loadEffectFromPath(std::string path);
	void getSourceNames();

	Steinberg::Vst::IAudioProcessor *getAudioProcessor()
	{
		return audioProcessor;
	}

	Steinberg::Vst::IEditController *getEditController()
	{
		return editController;
	}

public slots:
	void openEditor();
};

#endif // OBS_STUDIO_VST3PLUGIN_H
