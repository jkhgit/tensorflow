/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include "third_party/gpus/cuda/include/cuda.h"
#include "third_party/gpus/cuda/include/cusolverDn.h"
#include "tensorflow/compiler/xla/stream_executor/lib/env.h"
#include "tensorflow/compiler/xla/stream_executor/platform/dso_loader.h"

// Implements the cusolver API by forwarding to cusolver loaded from the DSO.

namespace {
// Returns DSO handle or null if loading the DSO fails.
void* GetDsoHandle() {
#ifdef PLATFORM_GOOGLE
  return nullptr;
#else
  static auto handle = []() -> void* {
    auto handle_or =
        stream_executor::internal::DsoLoader::GetCusolverDsoHandle();
    if (!handle_or.ok()) return nullptr;
    return handle_or.ValueOrDie();
  }();
  return handle;
#endif
}

template <typename T>
T LoadSymbol(const char* symbol_name) {
  void* symbol = nullptr;
  if (auto handle = GetDsoHandle()) {
    stream_executor::port::Env::Default()
        ->GetSymbolFromLibrary(handle, symbol_name, &symbol)
        .IgnoreError();
  }
  return reinterpret_cast<T>(symbol);
}

cusolverStatus_t GetSymbolNotFoundError() {
  return CUSOLVER_STATUS_INTERNAL_ERROR;
}
}  // namespace

#if CUDA_VERSION < 10000
#include "tensorflow/compiler/xla/stream_executor/cuda/cusolver_dense_9_0.inc"
#elif CUDA_VERSION < 10010
#include "tensorflow/compiler/xla/stream_executor/cuda/cusolver_dense_10_0.inc"
#elif CUDA_VERSION < 10020
#include "tensorflow/compiler/xla/stream_executor/cuda/cusolver_dense_10_1.inc"
#elif CUDA_VERSION < 11000
#include "tensorflow/compiler/xla/stream_executor/cuda/cusolver_dense_10_2.inc"
#else
#include "tensorflow/compiler/xla/stream_executor/cuda/cusolver_dense_11_0.inc"
#endif
