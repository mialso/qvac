#include "runtime/RunPipeline.hpp"

#include <stdexcept>

namespace qvac_lib_inference_addon_llama::runtime {

std::string RunPipeline::run(const RunRequest& request) const {
  if (!request.executeLegacyRun) {
    throw std::runtime_error("RunPipeline request is missing legacy executor");
  }

  return request.executeLegacyRun();
}

} // namespace qvac_lib_inference_addon_llama::runtime
