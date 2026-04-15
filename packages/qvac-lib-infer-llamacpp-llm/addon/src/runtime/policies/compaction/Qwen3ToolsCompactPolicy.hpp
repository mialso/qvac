#pragma once

#include "runtime/policies/compaction/CompactionPolicy.hpp"

namespace qvac_lib_inference_addon_llama::runtime {

class Qwen3ToolsCompactPolicy : public CompactionPolicy {
public:
  [[nodiscard]] bool toolsCompactEnabled() const override;
  [[nodiscard]] bool shouldEnforcePromptShape() const override;
  [[nodiscard]] bool shouldCaptureGeneratedOutput() const override;

  void onRunStart(llama_pos nPast, bool isCacheLoaded) override;
  void setConversationOnlyTokens(llama_pos tokens) override;
  void clearConversationOnlyTokens() override;
  void recordToolBoundary(llama_pos nPast, llama_pos totalTokens) override;

  [[nodiscard]] llama_pos
  clampDiscard(llama_pos nDiscarded, llama_pos firstMsgTokens) const override;
  void adjustAfterSlide(llama_pos discard, llama_pos firstMsgTokens) override;

  [[nodiscard]] bool
  hasDegenerateBoundary(llama_pos firstMsgTokens) const override;
  [[nodiscard]] bool
  hasUsableBoundary(llama_pos firstMsgTokens, llama_pos nPast) const override;
  [[nodiscard]] bool shouldTrimAfterGeneration(
      llama_pos firstMsgTokens, llama_pos nPast,
      const std::string& output) const override;
  [[nodiscard]] llama_pos nPastBeforeTools() const override;

  void reset() override;

private:
  llama_pos nConversationOnlyTokens_ = 0;
  llama_pos nPastBeforeTools_ = -1;
};

} // namespace qvac_lib_inference_addon_llama::runtime

