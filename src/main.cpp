#include <ui/camera_select.hpp>
#include <ui/capture_feed.hpp>

#include <fmt/format.h>

#include <gbBase/Log.hpp>
#include <gbBase/LogHandlers.hpp>

#include <escapi.h>

#pragma warning(push)
#pragma warning(disable: 5054)
#include <QApplication>
#include <QMessageBox>
#pragma warning(pop)

#include <filesystem>

int main(int argc, char* argv[])
{
    auto const gb_log_guard = Ghulbus::Log::initializeLoggingWithGuard();
    Ghulbus::Log::setLogHandler(Ghulbus::Log::Handlers::logToWindowsDebugger);

    QApplication the_app(argc, argv);
    if (!std::filesystem::exists("escapi.dll")) {
        QMessageBox msg_box(QMessageBox::Icon::Critical, "Could not find escapi.dll", "Could not find escapi.dll.");
        msg_box.exec();
        return 1;
    }

    setupESCAPI();

    ui::CameraSelect cam_select(nullptr);
    cam_select.refresh();
    cam_select.show();

    ui::CaptureFeed capture_feed(nullptr);
    QObject::connect(&cam_select, &ui::CameraSelect::captureStart, &capture_feed, &ui::CaptureFeed::onCaptureStart);

    return the_app.exec();
}
