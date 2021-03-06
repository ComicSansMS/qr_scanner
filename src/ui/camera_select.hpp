#ifndef QR_SCANNER_INCLUDE_GUARD_UI_CAMERA_SELECT_HPP
#define QR_SCANNER_INCLUDE_GUARD_UI_CAMERA_SELECT_HPP

#pragma warning(push)
#pragma warning(disable: 5054)
#include <QBoxLayout>
#include <QComboBox>
#include <QMenuBar>
#include <QPushButton>
#include <QWidget>
#pragma warning(pop)

namespace ui {

class CameraSelect : public QWidget {
    Q_OBJECT
private:
    QMenuBar* m_menuBar;
    QVBoxLayout* m_layout;
    QPushButton* m_refreshButton;
    QPushButton* m_startButton;
    QComboBox* m_camSelectCombo;
public:
    CameraSelect(QWidget* parent);

signals:
    void captureStart(int cam_index);
public slots:
    void refresh();
private slots:
    void onStartClicked();
};

}
#endif
