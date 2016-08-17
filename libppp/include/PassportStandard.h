#pragma once

#include <opencv2/core/core.hpp>
#include <rapidjson/document.h>
#include <memory>


/*!\class PassportStandard
Defines a passport photo dimensions specified by a country.
For example, Australia passport photos must be of at least 45mm x 35mm and the 
face height must be 34mm with +/- 2mm of tolerance
 */
class PassportStandard
{
private:
    double m_picHeight_mm; ///<- Height of the passport photo in mm
    double m_picWidth_mm;  ///<- Width of the passport photo in mm
    double m_faceHeight_mm; ///<- Height of the face in mm
public:
    PassportStandard(double picWidth_mm, double picHeight_mm, double faceHeight_mm);

    cv::Mat cropPicture(const cv::Mat& originalImage, const cv::Point& crownPoint, const cv::Point& chinPoint);

    double photoWidthMM() const { return m_picWidth_mm; }
    double photoHeightMM() const { return m_picHeight_mm; }

    static std::shared_ptr<PassportStandard> fromJson(rapidjson::Value& var);
};