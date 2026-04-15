#include "runtime/RunPipeline.hpp"

#include <stdexcept>

namespace qvac_lib_inference_addon_llama::runtime {

RunResult RunPipeline::run(const RunRequest& request) const {
  if (!request.executeLegacyRun) {
    throw std::runtime_error("RunPipeline request is missing legacy executor");
  }
  if (request.deps.context == nullptr) {
    throw std::runtime_error("RunPipeline request is missing runtime context");
  }

  return request.executeLegacyRun(request.deps);
}

} // namespace qvac_lib_inference_addon_llama::runtime
