#include <ui/camera_select.hpp>

#include <QApplication>
#include <QMessageBox>

#include <escapi.h>

namespace ui {

CameraSelect::CameraSelect(QWidget* parent)
    :QWidget(parent), m_menuBar(new QMenuBar(this)),
    m_layout(new QVBoxLayout(this)),
    m_refreshButton(new QPushButton("Refresh Cameras", this)),
    m_startButton(new QPushButton("Start Capture")),
    m_camSelectCombo(new QComboBox(this))
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowTitle("Select a camera to capture from");

    m_menuBar->addAction("Quit", [this]() { close(); });
    QMenu* about_menu = m_menuBar->addMenu("About");
    about_menu->addAction("About Qt", []() { QApplication::aboutQt(); });
    about_menu->addAction("About QR Scanner", []() {
            QMessageBox msg_box(QMessageBox::Icon::NoIcon, "About QR Scanner",
                                QString::fromUtf16(u" QR-Scanner\n\u00A9 2021 Andreas Weis\n\nThis software is licensed under GNU GPL.\nSee enclosed LICENSE file for details."));
            msg_box.exec();
        });
    m_layout->setMenuBar(m_menuBar);

    m_layout->addWidget(m_menuBar);
    m_layout->addWidget(m_camSelectCombo);
    m_layout->addWidget(m_refreshButton);
    m_layout->addWidget(m_startButton);
    setLayout(m_layout);

    //connect(m_aboutQtButton, &QPushButton::clicked, this, []() { QApplication::aboutQt(); });
    connect(m_refreshButton, &QPushButton::clicked, this, &CameraSelect::refresh);
    connect(m_startButton, &QPushButton::clicked, this, &CameraSelect::onStartClicked);
}

void CameraSelect::refresh()
{
    m_camSelectCombo->clear();
    int const n_cams = countCaptureDevices();
    char name_buffer[512];
    for (int i = 0; i < n_cams; ++i) {
        getCaptureDeviceName(i, name_buffer, sizeof(name_buffer));
        m_camSelectCombo->addItem(QString::fromUtf8(name_buffer));
    }
}

void CameraSelect::onStartClicked()
{
    emit captureStart(m_camSelectCombo->currentIndex());
    hide();
}

}
