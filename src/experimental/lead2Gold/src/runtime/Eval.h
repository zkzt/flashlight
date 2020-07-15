/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <flashlight/flashlight.h>

#include "criterion/criterion.h"
#include "data/W2lDataset.h"
#include "experimental/lead2Gold/src/runtime/Logger.h"
#include "libraries/common/Dictionary.h"
#include "runtime/runtime.h"

namespace w2l {
void evalOutput(
    const af::array& op,
    const af::array& target,
    std::map<std::string, fl::EditDistanceMeter>& mtr,
    const Dictionary& tgtDict,
    std::shared_ptr<SequenceCriterion> criterion);

void evalDataset(
    std::shared_ptr<fl::Module> ntwrk,
    std::shared_ptr<SequenceCriterion> criterion,
    std::shared_ptr<AutoSegBeamNoiseCriterion> asgbeamnoisecrit,
    std::shared_ptr<NoiseLMLetterSwapUnit> noiselm,
    int replabel,
    std::shared_ptr<W2lDataset> testds,
    SSLDatasetMeters& mtrs,
    DictionaryMap& dicts,
    bool evalbeamnoise = false);

void runEval(
    std::shared_ptr<fl::Module> network,
    std::shared_ptr<SequenceCriterion> criterion,
    std::shared_ptr<AutoSegBeamNoiseCriterion> asgbeamnoisecrit,
    std::shared_ptr<NoiseLMLetterSwapUnit> noiselm,
    int replabel,
    const std::unordered_map<std::string, std::shared_ptr<W2lDataset>>& ds,
    SSLTrainMeters& meters,
    DictionaryMap& dicts,
    bool evalbeamnoise = false);

} // namespace w2l
