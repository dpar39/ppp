#pragma once

#include "CommonHelpers.h"
#include <rapidjson/document.h>

/*!\class PhotoStandard
Defines a passport photo dimensions specified by a country.
For example, Australia passport photos must be of at least 45mm x 35mm and the
face height must be 34mm with +/- 2mm of tolerance
 */
FWD_DECL(PhotoStandard)

class PhotoStandard final
{
private:
    double m_picHeight_mm; ///<- Height of the passport photo in mm
    double m_picWidth_mm; ///<- Width of the passport photo in mm
    double m_faceHeight_mm; ///<- Height of the face in mm
    double m_eyesHeight_mm; ///<- Distance from the bottom of the picture to the eyes in mm (zero or negative means not
                            ///< provided)

public:
    PhotoStandard(double picWidth,
                  double picHeight,
                  double faceHeight,
                  double eyesHeight = 0.0,
                  const std::string & units = "mm");

    double photoWidthMM() const;

    double photoHeightMM() const;

    double faceHeightMM() const;

    double eyesHeightMM() const;

    static PhotoStandardSPtr fromJson(rapidjson::Value & photoStandardJson);
};
