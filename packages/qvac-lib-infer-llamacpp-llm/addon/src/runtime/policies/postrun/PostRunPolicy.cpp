#include "runtime/policies/postrun/PostRunPolicy.hpp"

#include "common/common.h"
#include "model-interface/CacheManager.hpp"
#include "model-interface/LlmContext.hpp"
#include "utils/LoggingMacros.hpp"

using namespace qvac_lib_inference_addon_cpp::logger;
using namespace qvac_lib_inference_addon_llama::logging;

namespace qvac_lib_inference_addon_llama::runtime {

PostRunResult PostRunPolicy::finalize(const PostRunRequest& request) const {
  PostRunResult result;

  auto& dts = request.context.dynamicToolsState();
  result.nPastBeforeTools = dts.nPastBeforeTools();
  const llama_pos firstMsgTokens = request.context.getFirstMsgTokens();

  if (dts.hasDegenerateToolBoundary(firstMsgTokens)) {
    QLOG_IF(
        Priority::WARNING,
        string_format(
            "[LlamaModel] tools_compact degenerate boundary at first message "
            "(nPastBeforeTools=%d, firstMsgTokens=%d); skipping "
            "post-generation tools trim\n",
            dts.nPastBeforeTools(),
            firstMsgTokens));
    dts.reset();
  }

  if (dts.hasUsableToolBoundary(firstMsgTokens) &&
      request.context.getNPast() > dts.nPastBeforeTools()) {
    const std::string& outputToCheck =
        request.outputCaptured ? request.capturedOutput : request.output;
    bool hasToolCall = outputToCheck.find("<tool_call>") != std::string::npos;
    if (!hasToolCall) {
      result.toolsTrimmed = true;
      request.context.removeLastNTokens(
          request.context.getNPast() - dts.nPastBeforeTools());
      dts.reset();
      if (request.context.getFirstMsgTokens() > request.context.getNPast()) {
        request.context.setFirstMsgTokens(request.context.getNPast());
      }
    }
  }

  if (request.saveCacheToDisk && request.cacheManager != nullptr &&
      request.cacheManager->hasActiveCache()) {
    request.cacheManager->saveCache();
  }

  if (request.shouldResetAfterInference && request.resetState) {
    request.resetState(false);
  }

  return result;
}

} // namespace qvac_lib_inference_addon_llama::runtime

