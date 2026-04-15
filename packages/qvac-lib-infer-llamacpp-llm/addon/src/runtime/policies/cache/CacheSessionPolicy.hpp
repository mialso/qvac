#pragma once

#include <string>
#include <vector>

#include "common/chat.h"
#include "runtime/RuntimeStateFacade.hpp"

namespace qvac_lib_inference_addon_llama::runtime {

struct CacheSessionResolution {
  std::vector<common_chat_msg> chatMsgs;
  std::vector<common_chat_tool> tools;
  bool isCacheLoaded = false;
  bool shouldResetAfterInference = true;
};

class CacheSessionPolicy {
public:
  CacheSessionResolution resolveSession(
      const std::string& inputPrompt, const std::string& cacheKey,
      const RuntimeDeps& deps) const;
};

} // namespace qvac_lib_inference_addon_llama::runtime

