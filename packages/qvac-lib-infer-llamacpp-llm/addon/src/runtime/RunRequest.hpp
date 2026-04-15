#pragma once

#include <functional>

#include "runtime/RuntimeStateFacade.hpp"

namespace qvac_lib_inference_addon_llama::runtime {

struct RunRequest {
  RuntimeDeps deps;
  std::function<RunResult(const RuntimeDeps&)> executeLegacyRun;
};

} // namespace qvac_lib_inference_addon_llama::runtime
