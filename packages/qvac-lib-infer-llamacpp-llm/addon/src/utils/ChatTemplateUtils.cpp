#include "ChatTemplateUtils.hpp"

#include <llama.h>

#include "profile/ModelProfile.hpp"
#include "utils/LoggingMacros.hpp"

using namespace qvac_lib_inference_addon_cpp::logger;

namespace qvac_lib_inference_addon_llama {
namespace utils {

bool isQwen3Model(const ::llama_model* model) {
  auto profile =
      qvac_lib_inference_addon_llama::profile::createProfileFromModel(model);
  return profile->capabilities().isQwen3;
}

std::string getChatTemplateForModel(
    const ::llama_model* model, const std::string& manualOverride,
    bool toolsCompact) {
  auto profile =
      qvac_lib_inference_addon_llama::profile::createProfileFromModel(model);
  return profile->selectChatTemplate(manualOverride, toolsCompact);
}

std::string getChatTemplate(
    const ::llama_model* model, const common_params& params,
    bool toolsCompact) {
  // Use fixed Qwen3 template if model is Qwen3 and Jinja is enabled
  std::string chatTemplate = params.chat_template;
  if (params.use_jinja) {
    chatTemplate =
        getChatTemplateForModel(model, params.chat_template, toolsCompact);
    if (!chatTemplate.empty() && chatTemplate != params.chat_template) {
      QLOG_IF(
          Priority::INFO, "[ChatTemplateUtils] Using fixed Qwen3 template\n");
    }
  }
  return chatTemplate;
}

std::string getPrompt(
    const struct common_chat_templates* tmpls,
    struct common_chat_templates_inputs& inputs) {
  try {
    return common_chat_templates_apply(tmpls, inputs).prompt;
  } catch (const std::exception& e) {
    // Catching known issue when a model does not support tools
    QLOG_IF(
        Priority::ERROR,
        string_format(
            "[ChatTemplateUtils] model does not support tools. Error: %s. "
            "Tools will "
            "be ignored.\n",
            e.what()));
    inputs.use_jinja = false;
    return common_chat_templates_apply(tmpls, inputs).prompt;
  } catch (...) {
    // Catching any other exception type
    QLOG_IF(
        Priority::ERROR,
        "[ChatTemplateUtils] model does not support tools (unknown exception). "
        "Tools "
        "will be ignored.\n");
    inputs.use_jinja = false;
    return common_chat_templates_apply(tmpls, inputs).prompt;
  }
}

} // namespace utils
} // namespace qvac_lib_inference_addon_llama
