#include <detect_qr.hpp>

#pragma warning(push)
#pragma warning(disable: 4251 5054)
#include <QZXing.h>
#pragma warning(pop)

QString detectQR(QImage const& image_to_decode)
{
    try {
        QZXing decoder;
        //mandatory settings
        decoder.setDecoder(QZXing::DecoderFormat_QR_CODE | QZXing::DecoderFormat_EAN_13);

        //optional settings
        //decoder.setSourceFilterType(QZXing::SourceFilter_ImageNormal | QZXing::SourceFilter_ImageInverted);
        decoder.setSourceFilterType(QZXing::SourceFilter_ImageNormal);
        decoder.setTryHarderBehaviour(QZXing::TryHarderBehaviour_ThoroughScanning | QZXing::TryHarderBehaviour_Rotate);

        //trigger decode
        QString result = decoder.decodeImage(image_to_decode);
        return result;
    } catch (...) {
        return "";
    }
}
