#ifndef OBS_STUDIO_EDITORDIALOG_H
#define OBS_STUDIO_EDITORDIALOG_H

#include <QWidget>

class VST3Plugin;

class EditorWidget : public QWidget {
	VST3Plugin *plugin;

public:
	EditorWidget(QWidget *parent, VST3Plugin *plugin);
	void buildEffectContainer();
};

#endif // OBS_STUDIO_EDITORDIALOG_H
