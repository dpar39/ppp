
#pragma once

#include <string>

class ArgsParser
{
public:
    ArgsParser(int argc, char ** argv);

private:
    // Input and output files
    std::string m_inputImage;
    std::string m_outputPrint;

    // Passport standard
    double m_picHeight; ///<- Height of the passport photo in mm
    double m_picWidth; ///<- Width of the passport photo in mm
    double m_faceHeight; ///<- Height of the face in mm
    double m_eyesHeight; ///<- Distance from the bottom of the picture to the eyes in mm (zero or negative means not
    std::string m_picUnits;
                            ///<provided)
    // Canvas specification
    double m_canvasWidth_mm; ///<- Output canvas width in mm
    double m_canvasHeight_mm; ///<- Output canvas height in mm
    double m_resolution_ppmm; ///<- Resolution in pixels per mm
    double m_border_mm; ///<- Separation between passport photos in the canvas in mm
    
};
