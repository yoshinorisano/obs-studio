#ifndef OBS_STUDIO_EDITORDIALOG_H
#define OBS_STUDIO_EDITORDIALOG_H

#include <QWidget>
#include <QWindow>
#include <Windows.h>

// TODO
#include "pluginterfaces/base/ipluginbase.h"

class VST3Plugin;

class EditorWidget : public QWidget {
	VST3Plugin *plugin;

	HWND windowHandle = NULL;

public:
	EditorWidget(QWidget *parent, VST3Plugin *plugin);
	void buildEffectContainer();
};

#endif // OBS_STUDIO_EDITORDIALOG_H
