#pragma once

#include <functional>
#include <string>

namespace qvac_lib_inference_addon_llama::runtime {

struct RunRequest {
  std::function<std::string()> executeLegacyRun;
};

} // namespace qvac_lib_inference_addon_llama::runtime
