#include <optional>
#include <string>

#include <gtest/gtest.h>

#include "model-interface/ModelMetadata.hpp"
#include "profile/ModelProfile.hpp"
#include "utils/Qwen3ToolsDynamicTemplate.hpp"
#include "utils/QwenTemplate.hpp"

namespace {

class MetadataStub : public ModelMetaData {
public:
  MetadataStub(std::optional<std::string> name, std::optional<std::string> arch)
      : name_(std::move(name)), arch_(std::move(arch)) {}

  std::optional<std::string> tryGetString(const char* key) const override {
    if (std::string(key) == "general.name") {
      return name_;
    }
    if (std::string(key) == "general.architecture") {
      return arch_;
    }
    return std::nullopt;
  }

private:
  std::optional<std::string> name_;
  std::optional<std::string> arch_;
};

} // namespace

using namespace qvac_lib_inference_addon_llama::profile;

TEST(ModelProfileTest, CreateFromNullModelReturnsDefaultCapabilities) {
  auto profile = createProfileFromModel(nullptr);
  ASSERT_NE(profile, nullptr);
  EXPECT_FALSE(profile->capabilities().isQwen3);
  EXPECT_FALSE(profile->capabilities().supportsToolsCompact);
}

TEST(ModelProfileTest, CreateFromMetadataUsesArchitectureDetection) {
  MetadataStub metadata(std::nullopt, "qwen3");
  auto profile = createProfileFromMetadata(metadata);
  ASSERT_NE(profile, nullptr);
  EXPECT_TRUE(profile->capabilities().isQwen3);
  EXPECT_TRUE(profile->capabilities().supportsToolsCompact);
}

TEST(ModelProfileTest, CreateFromMetadataUsesModelNameDetection) {
  MetadataStub metadata("Qwen-3-4B", std::nullopt);
  auto profile = createProfileFromMetadata(metadata);
  ASSERT_NE(profile, nullptr);
  EXPECT_TRUE(profile->capabilities().isQwen3);
  EXPECT_TRUE(profile->capabilities().supportsToolsCompact);
}

TEST(ModelProfileTest, DefaultProfileUsesManualTemplateOnly) {
  MetadataStub metadata(std::nullopt, "llama");
  auto profile = createProfileFromMetadata(metadata);
  ASSERT_NE(profile, nullptr);
  EXPECT_EQ(
      profile->selectChatTemplate("custom-template", false), "custom-template");
  EXPECT_EQ(profile->selectChatTemplate("", false), "");
  EXPECT_EQ(profile->selectChatTemplate("", true), "");
}

TEST(ModelProfileTest, Qwen3ProfileSelectsFixedTemplateWithoutToolsCompact) {
  MetadataStub metadata(std::nullopt, "qwen3");
  auto profile = createProfileFromMetadata(metadata);
  ASSERT_NE(profile, nullptr);
  EXPECT_EQ(
      profile->selectChatTemplate("", false),
      qvac_lib_inference_addon_llama::utils::getFixedQwen3Template());
}

TEST(ModelProfileTest, Qwen3ProfileSelectsDynamicTemplateWithToolsCompact) {
  MetadataStub metadata(std::nullopt, "qwen3");
  auto profile = createProfileFromMetadata(metadata);
  ASSERT_NE(profile, nullptr);
  EXPECT_EQ(
      profile->selectChatTemplate("", true),
      qvac_lib_inference_addon_llama::utils::getToolsDynamicQwen3Template());
}

TEST(ModelProfileTest, Qwen3ProfileManualOverrideStillWins) {
  MetadataStub metadata(std::nullopt, "qwen3");
  auto profile = createProfileFromMetadata(metadata);
  ASSERT_NE(profile, nullptr);
  EXPECT_EQ(profile->selectChatTemplate("manual-template", true), "manual-template");
}
