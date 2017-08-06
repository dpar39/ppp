#pragma once
#include <memory>
#include <string>
#include <vector>


#define FWD_DECL(classname) \
    class classname; \
    typedef std::shared_ptr<classname> classname##SPtr; \
    typedef std::unique_ptr<classname> classname##UPtr;

#define FWD_DECL_VEC(classname) \
    typedef std::vector<classname> classname##Vector; \
    typedef std::vector<std::shared_ptr<classname>> classname##SPtrVec; \
    typedef std::shared_ptr<std::vector<std::shared_ptr<classname>>> classname##SPtrVecSPtr;

#define FWD_DECL_LIST(classname) \
    typedef std::list<classname> classname##List; \
    typedef std::list<std::shared_ptr<classname>> classname##SPtrList; \
    typedef std::shared_ptr<std::list<std::shared_ptr<classname>>> classname##SPtrListcSPtr;

#define FWD_DECL_QUEUE(classname) \
    typedef std::deque<classname> classname##Deque; \
    typedef std::deque<std::shared_ptr<classname>> classname##SPtrDeque; \
    typedef std::shared_ptr<std::deque<std::shared_ptr<classname>>> classname##SPtrDequeSPtr;

namespace cv
{
    class CascadeClassifier;
}

typedef unsigned char byte;


class noncopyable
{
protected:
    noncopyable() = default;
    ~noncopyable() = default;
private:
    noncopyable(const noncopyable&) = delete;
    const noncopyable& operator=(const noncopyable&) = delete;
};

#if __cplusplus < 201300
namespace std
{
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif
