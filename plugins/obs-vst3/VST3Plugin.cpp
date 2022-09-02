#include "VST3Plugin.h"
#include "EditorWidget.h"

#include "obs.h"

#include <util/bmem.h>
#include <util/platform.h>
#include <windows.h>

// TODO
#include "pluginterfaces/base/ipluginbase.h"

typedef bool(PLUGIN_API *InitDllProc)();

VST3Plugin::VST3Plugin(obs_source_t *sourceContext)
	: sourceContext{sourceContext},
	  audioProcessor(nullptr),
	  editController(nullptr)
{
}

void VST3Plugin::openEditor()
{
	editorWidget = new EditorWidget(nullptr, this);
	editorWidget->buildEffectContainer();

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

void VST3Plugin::loadEffect()
{
	wchar_t *wpath;
	os_utf8_to_wcs_ptr(pluginPath.c_str(), 0, &wpath);
	dllHandle = LoadLibraryW(wpath);
	bfree(wpath);

	if (dllHandle == nullptr) {
		return;
	}

	GetFactoryProc factoryProc =
		(GetFactoryProc)GetProcAddress(dllHandle, "GetPluginFactory");

	if (factoryProc == nullptr) {
		return;
	}

	InitDllProc initDllProc =
		(InitDllProc)GetProcAddress(dllHandle, "InitDll");

	if (initDllProc == nullptr && !initDllProc()) {
		return;
	}

	Steinberg::IPluginFactory *pluginFactory = factoryProc();

	Steinberg::PFactoryInfo info;
	pluginFactory->getFactoryInfo(&info);

	blog(LOG_INFO, "obs-vst3: plugin vendor: %s, url: %s", info.vendor,
	     info.url);

	int count = pluginFactory->countClasses();
	for (int i = 0; i < count; i++) {
		Steinberg::PClassInfo classInfo;
		pluginFactory->getClassInfo(i, &classInfo);
		blog(LOG_INFO, "obs-vst3: class: %s, category: %s",
		     classInfo.name, classInfo.category);
		if (strncmp(classInfo.category, kVstAudioEffectClass,
			    strlen(kVstAudioEffectClass)) == 0) {
			Steinberg::tresult result = 0;
			Steinberg::Vst::IComponent *component = nullptr;
			result = pluginFactory->createInstance(
				classInfo.cid, Steinberg::Vst::IComponent::iid,
				(void **)&component);
			if (result != Steinberg::kResultOk) {
				return;
			}
			Steinberg::FUnknown *hostContext = nullptr; // TODO
			result = component->initialize(hostContext);
			if (result != Steinberg::kResultOk) {
				return;
			}

			result = component->queryInterface(
				Steinberg::Vst::IAudioProcessor::iid,
				(void **)&audioProcessor);
			if (result != Steinberg::kResultOk) {
				return;
			}

			audioProcessor->setProcessing(false);
			component->setActive(false);

			Steinberg::Vst::ProcessSetup setup;
			setup.processMode = Steinberg::Vst::kRealtime;
			setup.symbolicSampleSize = Steinberg::Vst::kSample32;
			setup.maxSamplesPerBlock = 512;
			setup.sampleRate = 44100.0;

			result = audioProcessor->setupProcessing(setup);
			if (result != Steinberg::kResultOk) {
				return;
			}

			component->setActive(true);
			audioProcessor->setProcessing(true);

			strcpy(this->effectName, classInfo.name);
		}
		if (strncmp(classInfo.category, kVstComponentControllerClass,
			    strlen(kVstComponentControllerClass)) == 0) {
			Steinberg::tresult result = 0;
			result = pluginFactory->createInstance(
				classInfo.cid,
				Steinberg::Vst::IEditController::iid,
				(void **)&editController);
			if (result != Steinberg::kResultOk) {
				return;
			}
			Steinberg::FUnknown *hostContext = nullptr; // TODO
			result = editController->initialize(hostContext);
			if (result != Steinberg::kResultOk) {
				return;
			}
		}
	}
}

void VST3Plugin::loadEffectFromPath(std::string path)
{
	pluginPath = path;
	loadEffect();
}

void VST3Plugin::getSourceNames()
{
	/* Only call inside the vst_filter_audio function! */
	sourceName = obs_source_get_name(obs_filter_get_parent(sourceContext));
	filterName = obs_source_get_name(sourceContext);
}

