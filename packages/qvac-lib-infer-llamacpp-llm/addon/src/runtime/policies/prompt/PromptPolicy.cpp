#include "runtime/policies/prompt/PromptPolicy.hpp"

#include <picojson/picojson.h>
#include <qvac-lib-inference-addon-cpp/Errors.hpp>
#include <llama/mtmd/mtmd.h>

#include "addon/LlmErrors.hpp"
#include "context/LlmContext.hpp"

using namespace qvac_lib_inference_addon_llama::errors;

namespace qvac_lib_inference_addon_llama::runtime {

// Keep legacy function name in formatted errors for compatibility.
constexpr const char* kPromptErrorFuncName = "formatPrompt";

std::pair<std::vector<common_chat_msg>, std::vector<common_chat_tool>>
PromptPolicy::resolvePrompt(
    const std::string& input, LlmContext& context, bool isTextLlm) const {
  if (input.empty()) {
    context.resetMedia();
    std::string errorMsg =
        string_format("%s: empty prompt\n", kPromptErrorFuncName);
    throw qvac_errors::StatusError(ADDON_ID, toString(EmptyPrompt), errorMsg);
  }

  std::vector<common_chat_msg> chatMsgs;
  std::vector<common_chat_tool> tools;

  picojson::value chatJson;
  std::string err = picojson::parse(chatJson, input);

  if (err.empty() && chatJson.is<picojson::array>()) {
    auto& obj = chatJson.get<picojson::array>();
    const bool toolsCompactEnabled = context.dynamicToolsState().toolsCompact();
    int64_t lastInputAnchorIndex = -1;
    int64_t firstToolIndex = -1;
    bool hasSplitToolBlock = false;
    bool hasNonToolAfterFirstTool = false;

    int addMediaPlaceholder = 0;
    bool isNextUser = false;
    for (size_t i = 0; i < obj.size(); ++i) {
      const auto& subObj = obj[i];
      if (!subObj.is<picojson::object>()) {
        continue;
      }

      picojson::object jsonObj = subObj.get<picojson::object>();
      if (jsonObj.find("type") != jsonObj.end() &&
          jsonObj["type"].get<std::string>() == "function") {
        if (firstToolIndex < 0) {
          firstToolIndex = static_cast<int64_t>(i);
        }
        if (hasNonToolAfterFirstTool) {
          hasSplitToolBlock = true;
        }

        common_chat_tool tool;
        tool.name = jsonObj["name"].get<std::string>();
        if (jsonObj.find("description") != jsonObj.end()) {
          tool.description = jsonObj["description"].get<std::string>();
        }
        if (jsonObj.find("parameters") != jsonObj.end()) {
          tool.parameters = jsonObj["parameters"].serialize();
        }
        tools.push_back(tool);
        continue;
      }

      common_chat_msg newMsg;
      if (jsonObj.find("role") == jsonObj.end()) {
        const char* errorMsg = "role is required in the input\n";
        throw qvac_errors::StatusError(
            ADDON_ID, toString(NoRoleProvided), errorMsg);
      }
      newMsg.role = jsonObj["role"].get<std::string>();
      if (newMsg.role == "user" || newMsg.role == "tool") {
        lastInputAnchorIndex = static_cast<int64_t>(i);
      }

      if (jsonObj.find("content") == jsonObj.end()) {
        const char* errorMsg = "content is required in the input\n";
        throw qvac_errors::StatusError(
            ADDON_ID, toString(NoContentProvided), errorMsg);
      }
      auto content = jsonObj["content"].get<std::string>();

      if (jsonObj.find("type") != jsonObj.end() &&
          jsonObj["type"].get<std::string>() == "media") {
        if (isTextLlm) {
          const char* errorMsg = "Media not supported by text-only models";
          throw qvac_errors::StatusError(
              ADDON_ID, toString(MediaNotSupported), errorMsg);
        }

        if (!content.empty()) {
          context.loadMedia(content);
        }
        addMediaPlaceholder++;
        isNextUser = true;
        continue;
      }

      if (newMsg.role == "user" && isNextUser) {
        isNextUser = false;
        while (addMediaPlaceholder > 0) {
          addMediaPlaceholder--;
          content.insert(0, mtmd_default_marker());
        }
      }
      if (newMsg.role != "user" && isNextUser) {
        context.resetMedia();
        std::string errorMsg = string_format(
            "%s: Must append a user question after loading media\n",
            kPromptErrorFuncName);
        throw qvac_errors::StatusError(
            ADDON_ID, toString(UserMessageNotProvided), errorMsg);
      }

      newMsg.content = content;
      chatMsgs.push_back(newMsg);
      if (firstToolIndex >= 0) {
        hasNonToolAfterFirstTool = true;
      }
    }

    if (toolsCompactEnabled) {
      if (tools.empty()) {
        std::string errorMsg = string_format(
            "%s: tools_compact requires non-empty tools attached to the last "
            "user message\n",
            kPromptErrorFuncName);
        throw qvac_errors::StatusError(
            ADDON_ID,
            qvac_errors::general_error::toString(
                qvac_errors::general_error::InvalidArgument),
            errorMsg);
      }
      if (lastInputAnchorIndex < 0) {
        std::string errorMsg = string_format(
            "%s: tools_compact requires a user or tool message before tools\n",
            kPromptErrorFuncName);
        throw qvac_errors::StatusError(
            ADDON_ID,
            qvac_errors::general_error::toString(
                qvac_errors::general_error::InvalidArgument),
            errorMsg);
      }
      if (hasSplitToolBlock || firstToolIndex != (lastInputAnchorIndex + 1)) {
        std::string errorMsg = string_format(
            "%s: tools_compact requires tools to be a contiguous block "
            "immediately after the last user or tool message\n",
            kPromptErrorFuncName);
        throw qvac_errors::StatusError(
            ADDON_ID,
            qvac_errors::general_error::toString(
                qvac_errors::general_error::InvalidArgument),
            errorMsg);
      }
    }

    if (addMediaPlaceholder > 0) {
      context.resetMedia();
      std::string errorMsg = string_format(
          "%s: No request for media was made\n", kPromptErrorFuncName);
      throw qvac_errors::StatusError(
          ADDON_ID, toString(MediaRequestNotProvided), errorMsg);
    }
  }

  if (!err.empty()) {
    context.resetMedia();
    std::string errorMsg = string_format(
        "%s: Invalid input format: %s\n",
        kPromptErrorFuncName,
        err.c_str());
    throw qvac_errors::StatusError(
        ADDON_ID, toString(InvalidInputFormat), errorMsg);
  }

  return {chatMsgs, tools};
}

} // namespace qvac_lib_inference_addon_llama::runtime

