#ifndef OBS_STUDIO_VST3PLUGIN_H
#define OBS_STUDIO_VST3PLUGIN_H

#include <string>
#include <QObject>
#include <Windows.h>

class EditorWidget;

class VST3Plugin : public QObject {
	Q_OBJECT

	EditorWidget *editorWidget = nullptr;
	std::string pluginPath;

	void *loadEffect();

	HINSTANCE dllHandle = nullptr;

public:
	void loadEffectFromPath(std::string path);

public slots:
	void openEditor();
};

#endif // OBS_STUDIO_VST3PLUGIN_H
