#ifndef OBS_STUDIO_VST3PLUGIN_H
#define OBS_STUDIO_VST3PLUGIN_H

#define BLOCK_SIZE 512

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

	float **inputs = nullptr;
	float **outputs = nullptr;
	float **channelrefs = nullptr;
	size_t numChannels = 0;
	void createChannelBuffers(size_t count);
	void cleanupChannelBuffers();

	void loadEffect();

	std::string sourceName;
	std::string filterName;
	char effectName[64];

	HINSTANCE dllHandle = nullptr;

	void unloadLibrary();

	Steinberg::Vst::IAudioProcessor *audioProcessor;
	Steinberg::Vst::IEditController *editController;

public:
	VST3Plugin(obs_source_t *sourceContext);
	~VST3Plugin();
	void loadEffectFromPath(std::string path);
	void unloadEffect();
	void getSourceNames();
	obs_audio_data *process(struct obs_audio_data *audio);

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
