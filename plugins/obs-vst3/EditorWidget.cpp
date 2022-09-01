#include "EditorWidget.h"
#include "VST3Plugin.h"

#include <util/base.h>

#include <QGridLayout>

// TODO
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/gui/iplugview.h"

static const char *VST3_CATEGORY_AUDIO_MODULE_CLASS = "Audio Module Class";
static const char *VST3_CATEGORY_CONTROLLER_CLASS =
	"Component Controller Class";


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
		if (strncmp(classInfo.category, VST3_CATEGORY_AUDIO_MODULE_CLASS,
			    strlen(VST3_CATEGORY_AUDIO_MODULE_CLASS)) == 0) {
			plugin->setEffectName(classInfo.name);
		}
		if (strncmp(classInfo.category, VST3_CATEGORY_CONTROLLER_CLASS,
			    strlen(VST3_CATEGORY_CONTROLLER_CLASS)) == 0) {
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

			Steinberg::IPlugView *view = editController->createView("editor");

			Steinberg::ViewRect size;
			result = view->getSize(&size);

			result = view->attached(windowHandle,
					     Steinberg::kPlatformTypeHWND);

			resize(size.getWidth(), size.getHeight());
		}
	}
}
