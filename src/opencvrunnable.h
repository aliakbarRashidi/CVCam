#ifndef OPENCVRUNNABLE_H
#define OPENCVRUNNABLE_H
#include <QVideoFilterRunnable>
#include <QVideoFrame>
#include <QImage>
#include <QDebug>

// Include OpenCV's C++ Interface
#include "opencv2/opencv.hpp"

class OpenCVFilter;
class OpenCVRunnable : public QVideoFilterRunnable
{
public:
    //QImage cvMatToQImage(const cv::Mat &inMat);
    //cv::Mat QImageToCvMat(const QImage &inImage, bool inCloneImageData=true);
    OpenCVRunnable(OpenCVFilter* filter);
protected:
    static void changeFacialSkinColor(cv::Mat smallImgBGR, cv::Mat bigEdges, int debugType);
    static void removePepperNoise(cv::Mat &mask);
    static void drawFaceStickFigure(cv::Mat dst);
    static void cartoonifyImage(cv::Mat inMat
                                , cv::Mat dst
                                , bool sketchMode=false
                                , bool alienMode=false
                                , bool evilMode=false
                                , int debugType=0);

    OpenCVFilter* filter;
};

class SketchVFRunnable : public OpenCVRunnable
{
public:
    SketchVFRunnable(OpenCVFilter* filter);
    virtual QVideoFrame run(QVideoFrame *input
                    , const QVideoSurfaceFormat &surfaceFormat
                    , RunFlags flags);
private:
    //QVideoFrame runWithImageWrapper(QVideoFrame *input
    //                    , const QVideoSurfaceFormat &surfaceFormat
    //                    , RunFlags flags);
};

// If inImage exists for the lifetime of the resulting cv::Mat, pass false to inCloneImageData to share inImage's
// data with the cv::Mat directly
//    NOTE: Format_RGB888 is an exception since we need to use a local QImage and thus must clone the data regardless
//    NOTE: This does not cover all cases - it should be easy to add new ones as required.
inline cv::Mat QImageToCvMat( const QImage &inImage, bool inCloneImageData )
{
   switch ( inImage.format() )
   {
      // 8-bit, 4 channel
      case QImage::Format_ARGB32:
      case QImage::Format_ARGB32_Premultiplied:
      {
         cv::Mat  mat( inImage.height(), inImage.width(),
                       CV_8UC4,
                       const_cast<uchar*>(inImage.bits()),
                       static_cast<size_t>(inImage.bytesPerLine())
                       );

         return (inCloneImageData ? mat.clone() : mat);
      }

      // 8-bit, 3 channel
      //case QImage::Format_Invalid: //temp try for android fix
      //  qWarning("Format invalid trying anyway");
      case QImage::Format_RGB32:
      case QImage::Format_RGB888:
      {
         if ( !inCloneImageData )
         {
            qWarning() << "ASM::QImageToCvMat() - Conversion requires cloning because we use a temporary QImage";
         }
         QImage   swapped = inImage;         
         //if (inImage.format() == QImage::Format_RGB32
         //     || inImage.format() == QImage::Format_Invalid)
         if (inImage.format() == QImage::Format_RGB32)
         {
            swapped = swapped.convertToFormat(QImage::Format_RGB888);
         }
         swapped = swapped.rgbSwapped();
         return cv::Mat( swapped.height(), swapped.width(),
                         CV_8UC3,
                         const_cast<uchar*>(swapped.bits()),
                         static_cast<size_t>(swapped.bytesPerLine())
                         ).clone();
      }

      // 8-bit, 1 channel
      case QImage::Format_Indexed8:
      {
         cv::Mat  mat( inImage.height(), inImage.width(),
                       CV_8UC1,
                       const_cast<uchar*>(inImage.bits()),
                       static_cast<size_t>(inImage.bytesPerLine())
                       );

         return (inCloneImageData ? mat.clone() : mat);
      }

      default:
         qWarning() << "ASM::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
         break;
   }

   return cv::Mat();
}

inline QImage cvMatToQImage(const cv::Mat &inMat)
{
    switch (inMat.type())
    {
        // 8-bit, 4 channel
    case CV_8UC4:
        {
            QImage image(inMat.data,inMat.cols, inMat.rows
                , static_cast<int>(inMat.step),
                QImage::Format_ARGB32);
            return image;
        }
    // 8-bit, 3 channel
    case CV_8UC3:
        {
            QImage image(inMat.data,
                inMat.cols, inMat.rows,
                static_cast<int>(inMat.step),
                QImage::Format_RGB888);
            return image.rgbSwapped();
        }
    // 8-bit, 1 channel
    case CV_8UC1:
        {
            // From Qt 5.5
            QImage image(inMat.data, inMat.cols, inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_Grayscale8);
        }

    default:
        qWarning() << "cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
        break;
    }
    return QImage();
}

#endif // OPENCVRUNNABLE_H