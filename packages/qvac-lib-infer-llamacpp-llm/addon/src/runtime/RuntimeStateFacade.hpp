#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "common/chat.h"

class CacheManager;
class LlmContext;

namespace qvac_lib_inference_addon_llama::runtime {

using PromptFormatter = std::function<std::pair<
    std::vector<common_chat_msg>,
    std::vector<common_chat_tool>>(const std::string&)>;

struct RuntimeDeps {
  LlmContext* context = nullptr;
  CacheManager* cacheManager = nullptr;
  PromptFormatter formatPrompt;
};

struct RunResult {
  std::string output;
};

} // namespace qvac_lib_inference_addon_llama::runtime
