#include <string>
#include "ArgsParser.h"

#define PARSE_ARG(ARG_NAME) \
    else if (std::strcmp(argv[idx], "--#x") == 0) \
        ARG_NAME = atof(argv[++idx]);

ArgsParser::ArgsParser(const int argc, char ** argv)


{
    // Canvas specification
    double m_canvasWidth_mm; ///<- Output canvas width in mm
    double m_canvasHeight_mm; ///<- Output canvas height in mm
    double m_resolution_ppmm; ///<- Resolution in pixels per mm
    double m_border_mm; ///<- Separation between passport photos 


    auto idx = 0;
    while (idx < argc)
    {



        if (std::strcmp(argv[idx], "--inputImage") == 0)
            m_inputImage = argv[++idx];
        if (std::strcmp(argv[idx], "--outImage") == 0)
            m_outputPrint = argv[++idx];
        
        // Passport standard
        else if (std::strcmp(argv[idx], "--pictureHeight") == 0)
            m_picHeight = atof(argv[++idx]);
        else if (std::strcmp(argv[idx], "--pictureWidth") == 0)
            m_picWidth = atof(argv[++idx]);
        else if (std::strcmp(argv[idx], "--faceHeight") == 0) m_faceHeight = atof(argv[++idx]);
        else if (std::strcmp(argv[idx], "--eyesHeight") == 0) m_eyesHeight = atof(argv[++idx]);
        else if (std::strcmp(argv[idx], "--pictureWidth") == 0) m_picWidth = atof(argv[++idx]);


        //// Print specs
        //else if (std::strcmp(argv[idx], "--canvasWidth") == 0)
        //    m_canvasWidth = argv[++idx];

        //else if (std::strcmp(argv[idx], "--pictureUnits") == 0)
        //    SASResLoc = argv[++idx];
        //else if (std::strcmp(argv[idx], "-locale") == 0)
        //    SASLocale = argv[++idx];
        //else if (std::strcmp(argv[idx], "-loglevel") == 0)
        //    LogLevel = argv[++idx];
        //else if (std::strcmp(argv[idx], "-logpath") == 0)
        //    LogPath = argv[++idx];
        //else if (std::strcmp(argv[idx], "-dpi") == 0)
        //    dpi = std::atoi(argv[++idx]);
        //else if (std::strcmp(argv[idx], "-width") == 0)
        //    width = std::atoi(argv[++idx]);
        //else if (std::strcmp(argv[idx], "-height") == 0)
        //    height = std::atoi(argv[++idx]);
        //else if (std::strcmp(argv[idx], "-mapfile") == 0)
        //    imageMap = argv[++idx];
        //idx++;



    }
}
