#include "runtime/policies/compaction/Qwen3ToolsCompactPolicy.hpp"

#include <algorithm>

namespace qvac_lib_inference_addon_llama::runtime {

bool Qwen3ToolsCompactPolicy::toolsCompactEnabled() const { return true; }

bool Qwen3ToolsCompactPolicy::shouldEnforcePromptShape() const { return true; }

bool Qwen3ToolsCompactPolicy::shouldCaptureGeneratedOutput() const {
  return true;
}

void Qwen3ToolsCompactPolicy::onRunStart(llama_pos nPast, bool isCacheLoaded) {
  if (nPast == 0 && !isCacheLoaded) {
    reset();
  }
}

void Qwen3ToolsCompactPolicy::setConversationOnlyTokens(llama_pos tokens) {
  nConversationOnlyTokens_ = tokens;
}

void Qwen3ToolsCompactPolicy::clearConversationOnlyTokens() {
  nConversationOnlyTokens_ = 0;
}

void Qwen3ToolsCompactPolicy::recordToolBoundary(
    llama_pos nPast, llama_pos totalTokens) {
  if (nConversationOnlyTokens_ > 0 && nPastBeforeTools_ == -1) {
    nPastBeforeTools_ = nPast - (totalTokens - nConversationOnlyTokens_);
  }
}

llama_pos Qwen3ToolsCompactPolicy::clampDiscard(
    llama_pos nDiscarded, llama_pos firstMsgTokens) const {
  if (nPastBeforeTools_ > firstMsgTokens) {
    llama_pos safeLimit = nPastBeforeTools_ - firstMsgTokens;
    return std::min(nDiscarded, safeLimit);
  }
  return nDiscarded;
}

void Qwen3ToolsCompactPolicy::adjustAfterSlide(
    llama_pos discard, llama_pos firstMsgTokens) {
  if (nPastBeforeTools_ > firstMsgTokens) {
    nPastBeforeTools_ -= discard;
  }
}

bool Qwen3ToolsCompactPolicy::hasDegenerateBoundary(
    llama_pos firstMsgTokens) const {
  return nPastBeforeTools_ == firstMsgTokens;
}

bool Qwen3ToolsCompactPolicy::hasUsableBoundary(
    llama_pos firstMsgTokens, llama_pos nPast) const {
  return nPastBeforeTools_ > 0 && nPastBeforeTools_ != firstMsgTokens &&
         nPast > nPastBeforeTools_;
}

bool Qwen3ToolsCompactPolicy::shouldTrimAfterGeneration(
    llama_pos firstMsgTokens, llama_pos nPast, const std::string& output) const {
  if (!hasUsableBoundary(firstMsgTokens, nPast)) {
    return false;
  }
  return output.find("<tool_call>") == std::string::npos;
}

llama_pos Qwen3ToolsCompactPolicy::nPastBeforeTools() const {
  return nPastBeforeTools_;
}

void Qwen3ToolsCompactPolicy::reset() {
  nConversationOnlyTokens_ = 0;
  nPastBeforeTools_ = -1;
}

} // namespace qvac_lib_inference_addon_llama::runtime

