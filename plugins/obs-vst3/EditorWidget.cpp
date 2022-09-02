#include "EditorWidget.h"
#include "VST3Plugin.h"

#include <util/base.h>

#include <QGridLayout>

// TODO
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/gui/iplugview.h"


EditorWidget::EditorWidget(QWidget *parent, VST3Plugin *plugin)
	: QWidget(parent), plugin(plugin)
{

}

void EditorWidget::buildEffectContainer(Steinberg::IPluginFactory *pluginFactory)
{
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
			result = pluginFactory->createInstance(classInfo.cid, Steinberg::Vst::IComponent::iid, (void **)&component);
			if (result != Steinberg::kResultOk) {
				return;
			}
			Steinberg::FUnknown *hostContext = nullptr; // TODO
			result = component->initialize(hostContext);
			if (result != Steinberg::kResultOk) {
				return;
			}

			Steinberg::Vst::IAudioProcessor *audioProcessor = nullptr;
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

			plugin->setEffectName(classInfo.name);
		}
		if (strncmp(classInfo.category, kVstComponentControllerClass,
			    strlen(kVstComponentControllerClass)) == 0) {
			Steinberg::tresult result = 0;
			Steinberg::Vst::IEditController *editController =
				nullptr;
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


			WNDCLASSEXW wcex{sizeof(wcex)};

			wcex.lpfnWndProc = DefWindowProcW;
			wcex.hInstance = GetModuleHandleW(nullptr);
			wcex.lpszClassName =
				L"Minimal VST host - Guest VST Window Frame";
			RegisterClassExW(&wcex);

			const auto style = WS_CAPTION | WS_THICKFRAME |
					   WS_OVERLAPPEDWINDOW;
			windowHandle = CreateWindowW(wcex.lpszClassName,
						     TEXT(""), style, 0, 0, 0,
						     0, nullptr, nullptr,
						     nullptr, nullptr);

			QWidget *widget = QWidget::createWindowContainer(
				QWindow::fromWinId((WId)windowHandle), nullptr);
			widget->move(0, 0);
			QGridLayout *layout = new QGridLayout();
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(0);
			setLayout(layout);
			layout->addWidget(widget);

			Steinberg::IPlugView *view = editController->createView(
				Steinberg::Vst::ViewType::kEditor);

			Steinberg::ViewRect size;
			result = view->getSize(&size);

			result = view->attached(windowHandle,
					     Steinberg::kPlatformTypeHWND);

			resize(size.getWidth(), size.getHeight());
		}
	}
}
