#pragma once

#include <string>

#include "llama.h"

namespace qvac_lib_inference_addon_llama::runtime {

class CompactionPolicy {
public:
  virtual ~CompactionPolicy() = default;

  [[nodiscard]] virtual bool toolsCompactEnabled() const = 0;
  [[nodiscard]] virtual bool shouldEnforcePromptShape() const = 0;
  [[nodiscard]] virtual bool shouldCaptureGeneratedOutput() const = 0;

  virtual void onRunStart(llama_pos nPast, bool isCacheLoaded) = 0;
  virtual void setConversationOnlyTokens(llama_pos tokens) = 0;
  virtual void clearConversationOnlyTokens() = 0;
  virtual void recordToolBoundary(llama_pos nPast, llama_pos totalTokens) = 0;

  [[nodiscard]] virtual llama_pos
  clampDiscard(llama_pos nDiscarded, llama_pos firstMsgTokens) const = 0;
  virtual void adjustAfterSlide(llama_pos discard, llama_pos firstMsgTokens) = 0;

  [[nodiscard]] virtual bool hasDegenerateBoundary(
      llama_pos firstMsgTokens) const = 0;
  [[nodiscard]] virtual bool hasUsableBoundary(
      llama_pos firstMsgTokens, llama_pos nPast) const = 0;
  [[nodiscard]] virtual bool shouldTrimAfterGeneration(
      llama_pos firstMsgTokens, llama_pos nPast,
      const std::string& output) const = 0;
  [[nodiscard]] virtual llama_pos nPastBeforeTools() const = 0;

  virtual void reset() = 0;
};

} // namespace qvac_lib_inference_addon_llama::runtime

