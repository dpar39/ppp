

// #include <onnxruntime/core/session/onnxruntime_cxx_api.h>

// namespace ppp
// {

// class OrtInstance
// {
// public:
//     ~OrtInstance();
//     explicit OrtInstance(const std::string & modelPath = "");

// private:
//     Ort::Env env_;
//     std::unique_ptr<Ort::Session> session_;
//     Ort::MemoryInfo memoryInfo_;
//     std::vector<int64_t> inputDims_;
//     std::vector<int64_t> outputDims_;

//     Ort::AllocatorWithDefaultOptions allocator_;
//     std::vector<char *> inputNames_;
//     std::vector<char *> outputNames_;
// };

// } // namespace ppp