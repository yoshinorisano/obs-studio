#ifndef OBS_STUDIO_VST3PLUGIN_H
#define OBS_STUDIO_VST3PLUGIN_H

#include <string>
#include <QObject>
#include <Windows.h>

#include "obs.h"

// TODO
#include "pluginterfaces/base/ipluginbase.h"

class EditorWidget;

class VST3Plugin : public QObject {
	Q_OBJECT

	EditorWidget *editorWidget = nullptr;
	obs_source_t *sourceContext;
	std::string pluginPath;

	Steinberg::IPluginFactory *pluginFactory = nullptr;
	Steinberg::IPluginFactory *loadEffect();

	std::string sourceName;
	std::string filterName;
	char effectName[64];

	HINSTANCE dllHandle = nullptr;

public:
	VST3Plugin(obs_source_t *sourceContext);
	void loadEffectFromPath(std::string path);
	void getSourceNames();

public slots:
	void openEditor();
};

#endif // OBS_STUDIO_VST3PLUGIN_H
