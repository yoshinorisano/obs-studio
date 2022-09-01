#include "VST3Plugin.h"
#include "EditorWidget.h"

#include <util/bmem.h>
#include <util/platform.h>
#include <windows.h>

// TODO
#include "pluginterfaces/base/ipluginbase.h"

typedef bool(PLUGIN_API *InitDllProc)();

void VST3Plugin::openEditor()
{
	editorWidget = new EditorWidget(nullptr, this);
	editorWidget->buildEffectContainer();
}

void *VST3Plugin::loadEffect()
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

	Steinberg::PFactoryInfo info;
	pluginFactory->getFactoryInfo(&info);

	blog(LOG_INFO, "obs-vst3: plugin vendor: %s, url: %s", info.vendor, info.url);

	return nullptr;
}

void VST3Plugin::loadEffectFromPath(std::string path)
{
	pluginPath = path;
	loadEffect();
}
