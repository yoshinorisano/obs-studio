#include "VST3Plugin.h"
#include "EditorWidget.h"

void VST3Plugin::openEditor()
{
	editorWidget = new EditorWidget(nullptr, this);
	editorWidget->buildEffectContainer();
}
