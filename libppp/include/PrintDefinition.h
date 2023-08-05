#pragma once

#include "CommonHelpers.h"
#include <rapidjson/document.h>
#include <string>

namespace cv {
class Mat;
}

namespace ppp {

FWD_DECL(PrintDefinition)

class PrintDefinition final
{
    double m_canvasWidth { 0.0 }; ///<- Output canvas width in mm
    double m_canvasHeight { 0.0 }; ///<- Output canvas height in mm
    mutable double m_resolution_dpi { 0.0 }; ///<- Resolution in pixels per mm
    double m_gutter { 0.0 }; ///<- Separation between passport photos in the canvas in mm
    double m_padding { 1.5 };

    std::string m_units; ///<- Units of the input dimensions

public:
    PrintDefinition(double width,
                    double height,
                    double dpi,
                    const std::string & units,
                    double gutter = 0.0,
                    double padding = 0.0);

    double height(const std::string & units = "pixel") const;

    double width(const std::string & units = "pixel") const;

    double gutter(const std::string & units = "pixel") const;

    double padding(const std::string & units = "pixel") const;

    double totalWidth(const std::string & units = "pixel") const;

    double totalHeight(const std::string & units = "pixel") const;

    // canvas:
    // {
    //     height: 6,
    //     width: 4,
    //     resolution: 300,
    //     border: 0.1,
    //     units: "inch"
    // }
    /*!@brief Construct a PrintDefinition from JSON data !*/
    static PrintDefinitionSPtr fromJson(rapidjson::Value & canvas);
    double resolutionDpi() const;

    void overrideResolution(double newDpi) const;
};
} // namespace ppp
