#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "model-interface/LlmContext.hpp"
#include "runtime/RuntimeStateFacade.hpp"
#include "runtime/policies/cache/CacheSessionPolicy.hpp"
#include "runtime/policies/generation/GenerationParamsPolicy.hpp"
#include "runtime/policies/postrun/PostRunPolicy.hpp"

namespace qvac_lib_inference_addon_llama::runtime {

struct RunRequest {
  RuntimeDeps deps;
  const CacheSessionPolicy* cacheSessionPolicy = nullptr;
  const GenerationParamsPolicy* generationParamsPolicy = nullptr;
  const PostRunPolicy* postRunPolicy = nullptr;

  std::string input;
  std::string cacheKey;
  bool prefill = false;
  bool saveCacheToDisk = false;
  GenerationParams generationParams;
  std::vector<std::vector<uint8_t>> media;
  std::function<void(const std::string&)> outputCallback;

  std::function<void(const std::vector<uint8_t>&)> loadMedia;
  std::function<void(bool)> resetState;
  std::function<void(llama_pos, bool)> setDebugBoundaryStats;
};

} // namespace qvac_lib_inference_addon_llama::runtime
