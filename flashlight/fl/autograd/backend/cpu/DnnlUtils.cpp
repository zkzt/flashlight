/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdexcept>

#include "flashlight/fl/autograd/backend/cpu/DnnlUtils.h"
#include "flashlight/fl/common/Defines.h"

namespace fl {
namespace detail {

dnnl::stream& DnnlStream::getStream() {
  return stream_;
}

DnnlStream& DnnlStream::getInstance() {
  static DnnlStream instance(DnnlEngine::getInstance().getEngine());
  return instance;
}

dnnl::engine& DnnlEngine::getEngine() {
  return engine_;
}

DnnlEngine& DnnlEngine::getInstance() {
  static DnnlEngine instance;
  return instance;
}

dnnl::memory::dims convertAfToDnnlDims(const std::vector<dim_t>& afDims) {
  // DNNL uses ints in dims
  std::vector<long int> intVec(afDims.begin(), afDims.end());
  return dnnl::memory::dims(intVec);
}

dnnl::memory dnnlAlignOrdering(
    std::vector<dnnl::primitive>& net,
    std::vector<std::unordered_map<int, dnnl::memory>>& netArgs,
    const dnnl::memory& memory,
    const dnnl::memory::desc& desc) {
  auto memoryOut = memory;
  if (memory.get_desc() != desc) {
    // use the ordering requested by the descriptor
    memoryOut =
        dnnl::memory(desc, detail::DnnlEngine::getInstance().getEngine());
    net.push_back(dnnl::reorder(memory, memoryOut));
    netArgs.push_back({{DNNL_ARG_FROM, memory}, {DNNL_ARG_TO, memoryOut}});
  }
  return memoryOut;
}

void executeNetwork(
    std::vector<dnnl::primitive>& net,
    std::vector<std::unordered_map<int, dnnl::memory>>& netArgs) {
  if (net.size() != netArgs.size()) {
    throw std::invalid_argument(
        "executeNetwork - given different size nets and netArgs");
  }
  bool cpuBackend = af::getActiveBackend() == AF_BACKEND_CPU;
  // If on the CPU backend, there isn't a AF computation stream that facilitates
  // enforcing that inputs to computation are ready; we're required to wait
  // until all AF operations are done
  if (cpuBackend) {
    af::sync();
  }
  for (size_t i = 0; i < net.size(); ++i) {
    net.at(i).execute(DnnlStream::getInstance().getStream(), netArgs.at(i));
  }
  if (cpuBackend) {
    // Block the executing thread until the work is complete
    DnnlStream::getInstance().getStream().wait();
  }
}

dnnl::algorithm dnnlMapToPoolingMode(const PoolingMode mode) {
  switch (mode) {
    case PoolingMode::MAX:
      return dnnl::algorithm::pooling_max;
    case PoolingMode::AVG_INCLUDE_PADDING:
      return dnnl::algorithm::pooling_avg_include_padding;
    case PoolingMode::AVG_EXCLUDE_PADDING:
      return dnnl::algorithm::pooling_avg_exclude_padding;
    default:
      throw std::invalid_argument("unsupported pooling mode for cuDNN");
  }
}

} // namespace detail
} // namespace fl
