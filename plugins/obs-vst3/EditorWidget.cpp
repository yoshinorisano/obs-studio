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

void EditorWidget::buildEffectContainer()
{
	Steinberg::tresult result = 0;
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

	Steinberg::IPlugView *view = plugin->getEditController()->createView(
		Steinberg::Vst::ViewType::kEditor);

	Steinberg::ViewRect size;
	result = view->getSize(&size);

	result = view->attached(windowHandle,
				Steinberg::kPlatformTypeHWND);

	resize(size.getWidth(), size.getHeight());
}
