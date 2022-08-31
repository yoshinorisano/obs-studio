#ifndef OBS_STUDIO_VST3PLUGIN_H
#define OBS_STUDIO_VST3PLUGIN_H

#include <QObject>

class VST3Plugin : public QObject {
	Q_OBJECT

public slots:
	void openEditor();
};

#endif // OBS_STUDIO_VST3PLUGIN_H
