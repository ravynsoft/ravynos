//===- tapi/SDKDB/PartailSDKDB.cpp - TAPI PartialSDKDB ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the Partial SDKDB
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/APIJSONSerializer.h"
#include "tapi/SDKDB/PartialSDKDB.h"

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

const char *PartialSDKDB::version = "1.0";

static void overwriteProjectNames(PartialSDKDB &result, StringRef name) {
  for (auto &api : result.binaryInterfaces) {
    if (api.getProjectName().empty())
      api.setProjectName(name);
  }
  for (auto &api : result.headerInterfaces) {
    if (api.getProjectName().empty())
      api.setProjectName(name);
  }
}

Expected<PartialSDKDB>
PartialSDKDB::createPublicAPIsFromJSON(json::Object &input) {
  PartialSDKDB output;
  // Parse RuntimeRoot..
  auto *runtimeRoot = input.getArray("RuntimeRoot");
  if (!runtimeRoot)
    return make_error<APIJSONError>("Missing RuntimeRoot");
  for (auto &entry : *runtimeRoot) {
    auto *api = entry.getAsObject();
    if (!api)
      return make_error<APIJSONError>("RuntimeRoot should be an array");

    if (auto result = APIJSONSerializer::parse(api))
      output.binaryInterfaces.emplace_back(std::move(*result));
    else
      return result.takeError();
  }
  // Parse PublicSDKContentRoot.
  auto *sdkRoot = input.getArray("PublicSDKContentRoot");
  if (!sdkRoot)
    return make_error<APIJSONError>("Missing PublicSDKContentRoot");
  for (auto &entry : *sdkRoot) {
    auto *api = entry.getAsObject();
    if (!api)
      return make_error<APIJSONError>("SDKContentRoot should be an array");

    if (auto result = APIJSONSerializer::parse(api))
      output.headerInterfaces.emplace_back(std::move(*result));
    else
      return result.takeError();
  }

  if (auto projectName = input.getString("projectName")) {
    output.project = projectName->str();
    overwriteProjectNames(output, *projectName);
  }

  if (auto hasError = input.getBoolean("error"))
    output.hasError = true;

  return output;
}

Expected<PartialSDKDB>
PartialSDKDB::createPrivateAPIsFromJSON(json::Object &input) {
  PartialSDKDB output;
  auto *runtimeRoot = input.getArray("RuntimeRoot");
  if (!runtimeRoot)
    return make_error<APIJSONError>("Missing RuntimeRoot");
  for (auto &entry : *runtimeRoot) {
    auto *api = entry.getAsObject();
    if (!api)
      return make_error<APIJSONError>("RuntimeRoot should be an array");

    if (auto result = APIJSONSerializer::parse(api))
      output.binaryInterfaces.emplace_back(std::move(*result));
    else
      return result.takeError();
  }
  // Parse SDKContentRoot and PublicSDKContentRoot.
  auto *publicSDKRoot = input.getArray("PublicSDKContentRoot");
  if (!publicSDKRoot)
    return make_error<APIJSONError>("Missing PublicSDKContentRoot");
  for (auto &entry : *publicSDKRoot) {
    auto *api = entry.getAsObject();
    if (!api)
      return make_error<APIJSONError>("SDKContentRoot should be an array");

    if (auto result = APIJSONSerializer::parse(api))
      output.headerInterfaces.emplace_back(std::move(*result));
    else
      return result.takeError();
  }

  auto *sdkRoot = input.getArray("SDKContentRoot");
  if (!sdkRoot)
    return make_error<APIJSONError>("Missing SDKContentRoot");
  for (auto &entry : *sdkRoot) {
    auto *api = entry.getAsObject();
    if (!api)
      return make_error<APIJSONError>("SDKContentRoot should be an array");

    if (auto result = APIJSONSerializer::parse(api))
      output.headerInterfaces.emplace_back(std::move(*result));
    else
      return result.takeError();
  }

  if (auto projectName = input.getString("projectName")) {
    output.project = projectName->str();
    overwriteProjectNames(output, *projectName);
  }

  if (auto hasError = input.getBoolean("error"))
    output.hasError = true;

  return output;
}

Error PartialSDKDB::serialize(
    llvm::raw_ostream &os, StringRef project,
    const std::vector<API> &binaryInterfaces,
    const std::vector<FrontendContext> &publicHeaderContext,
    const std::vector<API> &publicHeaderAPIs,
    const std::vector<FrontendContext> &privateHeaderContext,
    const std::vector<API> &privateHeaderAPIs, bool hasErrors,
    bool useCompatFormat) {
  // Write partial SDKDB.
  json::Object root;
  // Runtime Root.
  json::Array binaryInterfacesList;
  APIJSONOption options{
      /*compact*/ false,
      /*noUUID*/ true,
      /*noTarget*/ false,
      /*noHiddenGlobal*/ true,
      /*publicOnly*/ false,
      /*ignore line and col*/ true,
  };
  for (const auto &api : binaryInterfaces) {
    APIJSONSerializer serializer(api, options);
    binaryInterfacesList.emplace_back(serializer.getJSONObject());
  }
  root["RuntimeRoot"] = std::move(binaryInterfacesList);
  // SDKContentRoot Root.
  json::Array internalSDKInterfacesList;
  for (const auto &result : privateHeaderContext) {
    APIJSONSerializer serializer(*result.api, options);
    internalSDKInterfacesList.emplace_back(serializer.getJSONObject());
  }
  for (const auto &api : privateHeaderAPIs) {
    APIJSONSerializer serializer(api, options);
    internalSDKInterfacesList.emplace_back(serializer.getJSONObject());
  }
  root["SDKContentRoot"] = std::move(internalSDKInterfacesList);
  // PublicSDKContentRoot Root.
  json::Array publicSDKInterfacesList;
  for (const auto &result : publicHeaderContext) {
    APIJSONSerializer serializer(*result.api, options);
    publicSDKInterfacesList.emplace_back(serializer.getJSONObject());
  }
  for (const auto &api : publicHeaderAPIs) {
    APIJSONSerializer serializer(api, options);
    publicSDKInterfacesList.emplace_back(serializer.getJSONObject());
  }
  root["PublicSDKContentRoot"] = std::move(publicSDKInterfacesList);

  root["version"] = PartialSDKDB::version;
  if (hasErrors)
    root["error"] = true;

  if (!project.empty())
    root["projectName"] = project;

  if (useCompatFormat)
    os << formatv("{0}", json::Value(std::move(root))) << "\n";
  else
    os << formatv("{0:2}", json::Value(std::move(root))) << "\n";

  return Error::success();
}

TAPI_NAMESPACE_INTERNAL_END
