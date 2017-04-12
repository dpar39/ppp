#pragma once

#include <opencv2/core/core.hpp>
#include <rapidjson/document.h>
#include <memory>
#include "CommonHelpers.h"


/*!\class PhotoStandard
Defines a passport photo dimensions specified by a country.
For example, Australia passport photos must be of at least 45mm x 35mm and the
face height must be 34mm with +/- 2mm of tolerance
 */
FWD_DECL(PhotoStandard)

class PhotoStandard
{
private:
    double m_picHeight_mm; ///<- Height of the passport photo in mm
    double m_picWidth_mm;  ///<- Width of the passport photo in mm
    double m_faceHeight_mm;  ///<- Height of the face in mm
    double m_eyesHeight_mm;  ///<- Distance from the bottom of the picture to the eyes in mm (zero or negative means not provided)

public:
    PhotoStandard(double picWidth_mm, double picHeight_mm, double faceHeight_mm, double eyesHeight_mm = 0.0)
        : m_picHeight_mm(picHeight_mm)
        , m_picWidth_mm(picWidth_mm)
        , m_faceHeight_mm(faceHeight_mm)
        , m_eyesHeight_mm(eyesHeight_mm)
    {
    }

    double photoWidthMM() const { return m_picWidth_mm; }
    double photoHeightMM() const { return m_picHeight_mm; }
    double faceHeightMM() const { return m_faceHeight_mm; }
    double eyesHeightMM() const { return m_eyesHeight_mm; }

    static PhotoStandardSPtr fromJson(rapidjson::Value& var);
};