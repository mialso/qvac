#pragma once

#include <functional>
#include <string>

#include "llama.h"

class CacheManager;
class LlmContext;

namespace qvac_lib_inference_addon_llama::runtime {

struct PostRunRequest {
  LlmContext& context;
  CacheManager* cacheManager = nullptr;
  bool saveCacheToDisk = false;
  bool shouldResetAfterInference = false;
  bool outputCaptured = false;
  const std::string& output;
  const std::string& capturedOutput;
  std::function<void(bool)> resetState;
};

struct PostRunResult {
  llama_pos nPastBeforeTools = -1;
  bool toolsTrimmed = false;
};

class PostRunPolicy {
public:
  PostRunResult finalize(const PostRunRequest& request) const;
};

} // namespace qvac_lib_inference_addon_llama::runtime

