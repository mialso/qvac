#pragma once

#include <string>
#include <utility>
#include <vector>

#include "common/chat.h"

class LlmContext;

namespace qvac_lib_inference_addon_llama::runtime {

class PromptPolicy {
public:
  std::pair<std::vector<common_chat_msg>, std::vector<common_chat_tool>>
  resolvePrompt(const std::string& input, LlmContext& context, bool isTextLlm)
      const;
};

} // namespace qvac_lib_inference_addon_llama::runtime

