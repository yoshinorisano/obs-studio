#include "VST3Plugin.h"
#include "EditorWidget.h"

#include "obs.h"

#include <util/bmem.h>
#include <util/platform.h>
#include <windows.h>

// TODO
#include "pluginterfaces/base/ipluginbase.h"

typedef bool(PLUGIN_API *InitDllProc)();

VST3Plugin::VST3Plugin(obs_source_t *sourceContext) : sourceContext{sourceContext}
{
}

void VST3Plugin::openEditor()
{
	editorWidget = new EditorWidget(nullptr, this);
	editorWidget->buildEffectContainer(pluginFactory);

	if (sourceName.empty()) {
		sourceName = "VST 3.x";
	}

	if (filterName.empty()) {
		editorWidget->setWindowTitle(QString("%1 - %2").arg(
			sourceName.c_str(), effectName));
	} else {
		editorWidget->setWindowTitle(
			QString("%1: %2 - %3")
				.arg(sourceName.c_str(),
					filterName.c_str(), effectName));
	}

	editorWidget->show();
}

Steinberg::IPluginFactory *VST3Plugin::loadEffect()
{
	wchar_t *wpath;
	os_utf8_to_wcs_ptr(pluginPath.c_str(), 0, &wpath);
	dllHandle = LoadLibraryW(wpath);
	bfree(wpath);

	if (dllHandle == nullptr) {
		return nullptr;
	}

	GetFactoryProc factoryProc =
		(GetFactoryProc)GetProcAddress(dllHandle, "GetPluginFactory");

	if (factoryProc == nullptr) {
		return nullptr;
	}

	InitDllProc initDllProc =
		(InitDllProc)GetProcAddress(dllHandle, "InitDll");

	if (initDllProc == nullptr && !initDllProc()) {
		return nullptr;
	}

	Steinberg::IPluginFactory *pluginFactory = factoryProc();

	return pluginFactory;
	/*
	Steinberg::PFactoryInfo info;
	pluginFactory->getFactoryInfo(&info);

	blog(LOG_INFO, "obs-vst3: plugin vendor: %s, url: %s", info.vendor, info.url);

	int count = pluginFactory->countClasses();
	for (int i = 0; i < count; i++) {
		Steinberg::PClassInfo classInfo;
		pluginFactory->getClassInfo(i, &classInfo);
		blog(LOG_INFO, "obs-vst3: class: %s, category: %s", classInfo.name,
		     classInfo.category);
		if (strncmp(classInfo.category,
			    VST3_CATEGORY_CONTROLLER_CLASS, strlen(VST3_CATEGORY_CONTROLLER_CLASS)) == 0) {
			Steinberg::tresult result = 0;
			Steinberg::Vst::IEditController *editController = nullptr;
			result = pluginFactory->createInstance(classInfo.cid, Steinberg::Vst::IEditController::iid, (void **)&editController);
			if (result != Steinberg::kResultOk) {
				return nullptr;
			}
			//Steinberg::IPlugView *view = editController->createView("editor");
			//view->attached()


		}
	}

	return nullptr;
	*/
}

void VST3Plugin::loadEffectFromPath(std::string path)
{
	pluginPath = path;
	pluginFactory = loadEffect();
}

void VST3Plugin::getSourceNames()
{
	/* Only call inside the vst_filter_audio function! */
	sourceName = obs_source_get_name(obs_filter_get_parent(sourceContext));
	filterName = obs_source_get_name(sourceContext);
}

// TODO: Bad hack
void VST3Plugin::setEffectName(const char* effectName)
{
	strcpy(this->effectName, effectName);
}
