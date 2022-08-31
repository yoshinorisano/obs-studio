#ifndef OBS_STUDIO_VST3PLUGIN_H
#define OBS_STUDIO_VST3PLUGIN_H

#include <QObject>

class EditorWidget;

class VST3Plugin : public QObject {
	Q_OBJECT

	EditorWidget *editorWidget = nullptr;

public slots:
	void openEditor();
};

#endif // OBS_STUDIO_VST3PLUGIN_H
