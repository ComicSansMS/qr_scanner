#ifndef QR_SCANNER_INCLUDE_GUARD_UI_CAPTURE_FEED_HPP
#define QR_SCANNER_INCLUDE_GUARD_UI_CAPTURE_FEED_HPP

#pragma warning(push)
#pragma warning(disable: 4251 5054)
#include <QBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QImage>
#include <QLabel>
#include <QPalette>
#include <QPushButton>
#include <QTimer>
#include <QWidget>
#pragma warning(pop)

#include <memory>

namespace ui {

class CaptureFeed : public QWidget {
    Q_OBJECT
private:
    QVBoxLayout* m_layout;
    QLabel* m_imageLabel;
    QLineEdit* m_qrResult;
    QListWidget* m_resultList;
    QTimer* m_flashTimer;

    struct CaptureState;
    std::unique_ptr<CaptureState> m_state;
public:
    CaptureFeed(QWidget* parent);

    ~CaptureFeed() override;

signals:
    void feedImageBackChannel(QImage);
public slots:
    void onCaptureStart(int i);
private slots:
    void onImageBackChannel(QImage image);
};

}
#endif
