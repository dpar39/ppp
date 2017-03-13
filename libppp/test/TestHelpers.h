#pragma once

#include <fstream>

#ifdef POCO_STATIC
#include <Poco/DirectoryIterator.h>

inline void getImageFiles(const std::string &testImagesDir, std::vector<std::string> &imageFilenames)
{
    std::vector<std::string> supportedImageExtensions = { "jpg", /*"png",*/ "bmp" };

    using Poco::DirectoryIterator;
    using Poco::Path;

    DirectoryIterator itr(testImagesDir);
    DirectoryIterator end;
    while (itr != end)
    {
        if (!itr->isFile())
        {
            continue;
        }
        auto fileExt = Poco::Path(itr->path()).getExtension();
        std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);
        if (std::find(supportedImageExtensions.begin(), supportedImageExtensions.end(), fileExt) != supportedImageExtensions.end())
        {
            imageFilenames.push_back(itr->path());
        }
        ++itr;
    }
}

inline std::string resolvePath(const std::string &relPath)
{
    auto baseDir = Poco::Path(Poco::Path::current());
    while(baseDir.depth() > 1 )
    {
        auto currPath = baseDir;
        auto combinePath = Poco::File(currPath.append(relPath));
        if (combinePath.exists())
        {
            return combinePath.path();
        }
        baseDir = baseDir.parent();
    }
    return std::string();
}

inline std::string pathCombine(const std::string &prefix, const std::string &suffix)
{
    return Poco::Path(prefix, suffix);
}

#else
#include <filesystem>

inline std::string resolvePath(const std::string &relPath)
{
    namespace fs = std::tr2::sys;
    auto baseDir = fs::current_path();
    while (baseDir.has_parent_path())
    {
        auto combinePath = baseDir / relPath;
        if (fs::exists(combinePath))
        {
            return combinePath.string();
        }
        baseDir = baseDir.parent_path();
    }
    return std::string();
}

inline std::string pathCombine(const std::string &prefix, const std::string &suffix)
{
    namespace fs = std::tr2::sys;
    return (fs::path(prefix) / fs::path(suffix)).string();
}

inline void getImageFiles(const std::string &testImagesDir, std::vector<std::string> &imageFilenames)
{
    std::vector<std::string> supportedImageExtensions = { ".jpg", /*".png",*/ ".bmp" };

    namespace fs = std::tr2::sys;
    fs::directory_iterator endIter;
    if (fs::exists(fs::path(testImagesDir)) && fs::is_directory(fs::path(testImagesDir)))
    {
        for (fs::directory_iterator itr(testImagesDir); itr != endIter; ++itr)
        {
            if (!fs::is_regular_file(itr->status()))
            {
                continue;
            }
            auto filePath = itr->path();
            auto fileExt = filePath.extension().string();
            std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);
            if (std::find(supportedImageExtensions.begin(), supportedImageExtensions.end(), fileExt) != supportedImageExtensions.end())
            {
                imageFilenames.push_back(filePath.string());
            }
        }
    }
}
#endif

inline bool importTextMatrix(const std::string&txtFileName, cv::Mat &output)
{
    std::ifstream textFile(txtFileName);
    std::string str;
    int numRows = 0;
    int numCols = 0;
    std::vector<float> data;

    while (getline(textFile, str))
    {
        std::stringstream stream(str);
        int numValuesInRow = 0;
        while (1)
        {
            float value;
            stream >> value;
            if (!stream)
                break;
            data.push_back(value);
            numValuesInRow++;
        }
        if (numRows == 0)
        {
            numCols = numValuesInRow;
        }
        else if (numValuesInRow == 0)
        {
            break; // Empty line
        }
        else if (numCols != numValuesInRow)
        {
            textFile.close();
            return false;
        }
        numRows++;
    }
    auto dataPtr = data.data();
    output = cv::Mat(numRows, numCols, CV_32F, dataPtr, cv::Mat::AUTO_STEP).clone();
    textFile.close();
    return true;
}

