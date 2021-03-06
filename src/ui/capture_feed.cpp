#include <ui/capture_feed.hpp>

#include <detect_qr.hpp>

#include <gbBase/Assert.hpp>

#include <escapi.h>

#include <QApplication>
#include <QClipboard>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace ui {

namespace {
constexpr int CAPTURE_WIDTH = 1280;
constexpr int CAPTURE_HEIGHT = 720;
}

struct CaptureFeed::CaptureState {
public:
    using CaptureCallback = std::function<void(QImage const&)>;
private:
    int m_activeCam;
    std::vector<int> m_captureBuffer;
    std::mutex m_mtx;
    std::condition_variable m_cv;
    bool m_quitRequested;
    std::thread m_captureThread;
    CaptureCallback m_captureCallback;
public:

    CaptureState();
    ~CaptureState();

    void startCapture(int i);

    void setCaptureCallback(CaptureCallback const& cb);

private:
    void captureThread();
};

CaptureFeed::CaptureState::CaptureState()
    :m_activeCam(-1), m_quitRequested(false), m_captureThread([this]() { captureThread(); })
{}

CaptureFeed::CaptureState::~CaptureState()
{
    {
        std::scoped_lock lk(m_mtx);
        m_quitRequested = true;
    }
    m_cv.notify_all();
    m_captureThread.join();

    if (m_activeCam != -1) {
        deinitCapture(m_activeCam);
    }
}

void CaptureFeed::CaptureState::startCapture(int i)
{
    std::scoped_lock lk(m_mtx);
    if (m_activeCam != -1) {
        // todo
        return;
    }
    GHULBUS_PRECONDITION(i >= 0);
    SimpleCapParams cp;
    cp.mWidth = CAPTURE_WIDTH;
    cp.mHeight = CAPTURE_HEIGHT;
    m_captureBuffer.resize(CAPTURE_WIDTH * CAPTURE_HEIGHT);
    cp.mTargetBuf = m_captureBuffer.data();
    if (initCapture(i, &cp) == 1) {
        m_activeCam = i;
        m_cv.notify_one();
    }
}

void CaptureFeed::CaptureState::captureThread()
{
    for (;;)
    {
        int active_cam = -1;
        std::unique_lock lk(m_mtx);
        m_cv.wait(lk, [this]() { return m_quitRequested || (m_activeCam >= 0); });
        if (m_quitRequested) { return; }
        active_cam = m_activeCam;
        GHULBUS_ASSERT(active_cam >= 0);
        doCapture(active_cam);
        while (isCaptureDone(active_cam) == 0) {
            if (m_quitRequested) { return; }
            lk.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            lk.lock();
        }
        // capture done, emit image
        if (m_captureCallback) {
            QImage image(CAPTURE_WIDTH, CAPTURE_HEIGHT, QImage::Format_RGB32);
            for (int iy = 0; iy < image.height(); ++iy) {
                for (int ix = 0; ix < image.width(); ++ix) {
                    image.setPixel(ix, iy, m_captureBuffer[iy*CAPTURE_WIDTH + ix]);
                }
            }
            auto const local_cb = m_captureCallback;
            lk.unlock();
            local_cb(image);
            lk.lock();
        }
    }
}

void CaptureFeed::CaptureState::setCaptureCallback(CaptureCallback const& cb)
{
    std::scoped_lock lk(m_mtx);
    m_captureCallback = cb;
}

CaptureFeed::CaptureFeed(QWidget* parent)
    :QWidget(parent), m_layout(new QVBoxLayout(this)), m_imageLabel(new QLabel(this)), m_qrResult(new QLineEdit(this)),
     m_resultList(new QListWidget(this)), m_flashTimer(new QTimer(this)),
     m_state(std::make_unique<CaptureState>())
{
    setWindowTitle("Scanning for QR Codes");
    m_qrResult->setEnabled(false);

    m_layout->addWidget(m_imageLabel);
    m_layout->addWidget(m_qrResult);
    m_layout->addWidget(m_resultList);
    setLayout(m_layout);

    connect(this, &CaptureFeed::feedImageBackChannel, this, &CaptureFeed::onImageBackChannel, Qt::ConnectionType::QueuedConnection);
    m_state->setCaptureCallback([this](QImage const& image) { emit feedImageBackChannel(image); });

    connect(m_resultList, &QListWidget::doubleClicked, this, [this](QModelIndex const& i) {
                QString const r = m_resultList->item(i.row())->text();
                QApplication::clipboard()->setText(r);
            });

    connect(m_flashTimer, &QTimer::timeout, [palette = palette(), this]() { setPalette(palette); });
}

CaptureFeed::~CaptureFeed() = default;

void CaptureFeed::onCaptureStart(int i)
{
    show();
    m_state->startCapture(i);
}

void CaptureFeed::onImageBackChannel(QImage image)
{
    m_imageLabel->setPixmap(QPixmap::fromImage(image));
    QString detected = detectQR(image);
    if (!detected.isEmpty()) {
        m_qrResult->setText(detected);
        bool is_duplicate = false;
        for (int i = 0; i < m_resultList->count(); ++i) {
            if (m_resultList->item(i)->text() == detected) {
                is_duplicate = true;
            }
        }
        if (!is_duplicate) { m_resultList->insertItem(0, detected); }
        QApplication::clipboard()->setText(detected);
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::darkBlue);
        setPalette(p);
        m_flashTimer->start(std::chrono::milliseconds(1000));
    }
}
}
