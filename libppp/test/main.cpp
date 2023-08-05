#include <gtest/gtest.h>

#include <glog/logging.h>

int main(int argc, char ** argv)
{
    FLAGS_minloglevel = google::GLOG_WARNING;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
