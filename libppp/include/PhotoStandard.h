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
    double m_photoHeight; ///<- Height of the photo [mm]
    double m_photoWidth; ///<- Width of the photo [mm]
    double m_faceHeight; ///<- Height of the face (crown to chin distance) [mm]
    double m_crownTop; ///<- Distance from the top of the photo to the crown [mm] (zero if not provided)
    mutable double m_resolution_dpi; ///<- Standard required print resolution (dots per inch)

    std::string m_units;

public:
    PhotoStandard(double photoWidth,
                  double photoHeight,
                  double faceHeight,
                  double crownTop,
                  double eyeLineBottom,
                  double picResolution,
                  const std::string & units = "mm");

    double photoWidth(const std::string & units = "pixel") const;

    double photoHeight(const std::string & units = "pixel") const;

    double faceHeight(const std::string & units = "pixel") const;

    double crownTop(const std::string & units = "pixel") const;

    double resolutionDpi() const;

    void overrideResolution(double newDpi) const;

    static PhotoStandardSPtr fromJson(const rapidjson::Value & photoStandardJson);
    static PhotoStandardSPtr fromJson(const std::string & photoStandardJson);
};
} // namespace ppp
