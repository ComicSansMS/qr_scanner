#ifndef QR_SCANNER_INCLUDE_GUARD_DETECT_QR_HPP
#define QR_SCANNER_INCLUDE_GUARD_DETECT_QR_HPP

#pragma warning(push)
#pragma warning(disable: 5054)
#include <QImage>
#include <QString>
#pragma warning(pop)

QString detectQR(QImage const& image_to_decode);

#endif
