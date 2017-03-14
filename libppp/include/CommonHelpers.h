#pragma once
#include <memory>
#include <string>

namespace cv
{
    class CascadeClassifier;
}

class CommonHelpers
{
public:
    /*!@brief Loads a cascade classifier from file
    *  @param[in] haarCascadeBase64Data Haar cascade XML data encoded as a base64 string
    *  @returns The classifier loaded into memory
    !*/
    //static std::shared_ptr<cv::CascadeClassifier> loadClassifierFromFile(const std::string &haarCascadeDir, const std::string &haarCascadeFile);
    
    static std::shared_ptr<cv::CascadeClassifier> loadClassifierFromBase64(const std::string &haarCascadeBase64Data);

    /*!@brief Calculates CRC value for a buffer of specified length !*/
    static uint32_t crc32(uint32_t crc, const uint8_t* begin, const uint8_t* end);


    /*!@brief Convert a distance to millimeters
     * @param[in] v Value to convert
     * @param[in] units Units (accepted values are "inch", "mm" and "cm"
    !*/
    static double toMM(double v, const std::string &units);
};

class noncopyable
{
protected:
    noncopyable() = default;
    ~noncopyable() = default;
private:
    noncopyable(const noncopyable&) = delete;
    const noncopyable& operator=(const noncopyable&) = delete;
};

namespace std 
{
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}

