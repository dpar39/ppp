#pragma once
#include <memory>

#define FWD_DECL(classname)                                                                                            \
    class classname;                                                                                                   \
    typedef std::shared_ptr<classname> classname##SPtr;                                                                \
    typedef std::unique_ptr<classname> classname##UPtr;

#define FWD_DECL_VEC(classname)                                                                                        \
    typedef std::vector<classname> classname##Vector;                                                                  \
    typedef std::vector<std::shared_ptr<classname>> classname##SPtrVec;                                                \
    typedef std::shared_ptr<std::vector<std::shared_ptr<classname>>> classname##SPtrVecSPtr;

#define FWD_DECL_LIST(classname)                                                                                       \
    typedef std::list<classname> classname##List;                                                                      \
    typedef std::list<std::shared_ptr<classname>> classname##SPtrList;                                                 \
    typedef std::shared_ptr<std::list<std::shared_ptr<classname>>> classname##SPtrListcSPtr;

#define FWD_DECL_QUEUE(classname)                                                                                      \
    typedef std::deque<classname> classname##Deque;                                                                    \
    typedef std::deque<std::shared_ptr<classname>> classname##SPtrDeque;                                               \
    typedef std::shared_ptr<std::deque<std::shared_ptr<classname>>> classname##SPtrDequeSPtr;

namespace cv
{
FWD_DECL(CascadeClassifier);
}

using BYTE = uint8_t;

class NonCopyable
{
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;

public:
    NonCopyable(const NonCopyable &) = delete;
    const NonCopyable & operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable &&) = delete;
    const NonCopyable & operator=(NonCopyable &&) = delete;
};

#if __cplusplus < 201300
namespace std
{
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
} // namespace std

// String literals
#endif

#define DEFINE_STR(keyName, valString) constexpr auto keyName = #valString;

DEFINE_STR(IMAGE_ID, imgKey)
DEFINE_STR(PRINT_DEFINITION, canvas)
DEFINE_STR(PHOTO_STANDARD, standard)
DEFINE_STR(CROWN_POINT, crownPoint)
DEFINE_STR(CHIN_POINT, chinPoint)
DEFINE_STR(EXIF_INFO, EXIFInfo);
DEFINE_STR(AS_BASE64, asBase64);

DEFINE_STR(UNITS, units);

// Photo requirement fields
DEFINE_STR(PHOTO_WIDTH, pictureWidth);
DEFINE_STR(PHOTO_HEIGHT, pictureHeight);
DEFINE_STR(PHOTO_FACE_HEIGHT, faceHeight);
DEFINE_STR(PHOTO_CROWN_TOP, crownTop);
DEFINE_STR(PHOTO_EYELINE_BOTTOM, bottomEyeLine);
DEFINE_STR(PHOTO_RESOLUTION, dpi);

DEFINE_STR(COMPLIANCE_CHECKS, complianceChecks);
