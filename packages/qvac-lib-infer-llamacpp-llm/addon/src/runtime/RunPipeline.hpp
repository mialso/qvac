#pragma once

#include "runtime/RunRequest.hpp"

namespace qvac_lib_inference_addon_llama::runtime {

class RunPipeline {
public:
  RunResult run(const RunRequest& request) const;
};

} // namespace qvac_lib_inference_addon_llama::runtime
