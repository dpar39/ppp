#pragma once

#include "CommonHelpers.h"
#include <rapidjson/document.h>

namespace ppp
{
/*!\class PhotoStandard
Defines a passport photo dimensions specified by a country.
For example, Australia passport photos must be of at least 45mm x 35mm and the
face height must be 34mm with +/- 2mm of tolerance
 */
FWD_DECL(PhotoStandard)

class PhotoStandard final
{
private:
    double m_picHeight_mm; ///<- Height of the photo [mm]
    double m_picWidth_mm; ///<- Width of the photo [mm]
    double m_faceHeight_mm; ///<- Height of the face (crown to chin distance) [mm]
    double m_crownTop_mm; ///<- Distance from the top of the photo to the crown [mm] (zero if not provided)
    double m_printResolution_dpi; ///<- Standard required print resolution (dots per inch)

public:
    PhotoStandard(double picWidth,
                  double picHeight,
                  double faceHeight,
                  double crownTop,
                  const double eyeLineBottom,
                  double printResolution,
                  const std::string & units = "mm");

    double photoWidthMM() const;

    double photoHeightMM() const;

    double faceHeightMM() const;

    double crownTopMM() const;

    static PhotoStandardSPtr fromJson(const rapidjson::Value & photoStandardJson);
};
} // namespace ppp
