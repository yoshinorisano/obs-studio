#include "VST3Plugin.h"
#include "EditorWidget.h"

#include "obs.h"

#include <util/bmem.h>
#include <util/platform.h>
#include <windows.h>
#include <assert.h>

// TODO
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/vsttypes.h"

typedef bool(PLUGIN_API *InitDllProc)();

VST3Plugin::VST3Plugin(obs_source_t *sourceContext)
	: sourceContext{sourceContext},
	  audioProcessor(nullptr),
	  editController(nullptr)
{
}

VST3Plugin::~VST3Plugin()
{
	unloadEffect();
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

	int maxchans = 2; // TODO
	createChannelBuffers(maxchans);
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

void VST3Plugin::createChannelBuffers(size_t count)
{
	cleanupChannelBuffers();

	int blocksize = BLOCK_SIZE;
	numChannels = (std::max)((size_t)0, count);

	if (numChannels > 0) {
		inputs = (float **)malloc(sizeof(float *) * numChannels);
		outputs = (float **)malloc(sizeof(float *) * numChannels);
		channelrefs = (float **)malloc(sizeof(float *) * numChannels);
		for (size_t channel = 0; channel < numChannels; channel++) {
			inputs[channel] =
				(float *)malloc(sizeof(float) * blocksize);
			outputs[channel] =
				(float *)malloc(sizeof(float) * blocksize);
		}
	}
}

void VST3Plugin::cleanupChannelBuffers()
{
	for (size_t channel = 0; channel < numChannels; channel++) {
		if (inputs && inputs[channel]) {
			free(inputs[channel]);
			inputs[channel] = NULL;
		}
		if (outputs && outputs[channel]) {
			free(outputs[channel]);
			outputs[channel] = NULL;
		}
	}
	if (inputs) {
		free(inputs);
		inputs = NULL;
	}
	if (outputs) {
		free(outputs);
		outputs = NULL;
	}
	if (channelrefs) {
		free(channelrefs);
		channelrefs = NULL;
	}
	numChannels = 0;
}

static void silenceChannel(float **channelData, size_t numChannels,
			   long numFrames)
{
	for (size_t channel = 0; channel < numChannels; ++channel) {
		for (long frame = 0; frame < numFrames; ++frame) {
			channelData[channel][frame] = 0.0f;
		}
	}
}

obs_audio_data *VST3Plugin::process(struct obs_audio_data *audio)
{
	assert(audioProcessor != nullptr);
	Steinberg::tresult result = 0;

	uint passes = (audio->frames + BLOCK_SIZE - 1) / BLOCK_SIZE;
	uint extra = audio->frames % BLOCK_SIZE;
	for (uint pass = 0; pass < passes; pass++) {
		uint frames = pass == passes - 1 && extra ? extra : BLOCK_SIZE;

		silenceChannel(outputs, numChannels, BLOCK_SIZE);

		for (size_t d = 0; d < numChannels; d++) {
			if (d < MAX_AV_PLANES && audio->data[d] != nullptr) {
				channelrefs[d] = ((float *)audio->data[d]) +
						 (pass * BLOCK_SIZE);
			} else {
				channelrefs[d] = inputs[d];
			}
		}

		Steinberg::Vst::AudioBusBuffers inputBuffers;
		inputBuffers.numChannels = (Steinberg::int32)numChannels;
		inputBuffers.channelBuffers32 =
			(Steinberg::Vst::Sample32 **)channelrefs;

		Steinberg::Vst::AudioBusBuffers outputBuffers;
		outputBuffers.numChannels = (Steinberg::int32)numChannels;
		outputBuffers.channelBuffers32 =
			(Steinberg::Vst::Sample32 **)outputs;

		Steinberg::Vst::ProcessData data;
		data.processMode = Steinberg::Vst::ProcessModes::kRealtime;
		data.symbolicSampleSize =
			Steinberg::Vst::SymbolicSampleSizes::kSample32;
		data.numSamples = frames;
		data.numInputs = 1;
		data.numOutputs = 1;
		data.inputs = &inputBuffers;
		data.outputs = &outputBuffers;

		result = audioProcessor->process(data);
		if (result != Steinberg::kResultOk) {
			return nullptr; // TODO
		}

		// only copy back the channels the plugin may have generated
		for (size_t c = 0; c < numChannels && c < MAX_AV_PLANES; c++) {
			if (audio->data[c]) {
				for (size_t i = 0; i < frames; i++) {
					channelrefs[c][i] = outputs[c][i];
				}
			}
		}
	}

	return audio;
}

void VST3Plugin::unloadEffect()
{
	unloadLibrary();
}

void VST3Plugin::unloadLibrary()
{
	if (dllHandle) {
		FreeLibrary(dllHandle);
		dllHandle = nullptr;
	}
}
