/***************************************************************************************
* Copyright (c) 2020-2023 Institute of Computing Technology, Chinese Academy of Sciences
*
* DiffTest is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "state.h"
#include "refproxy.h"

#ifdef CONFIG_STATE_ARCHINTREG

ArchIntRegStateTracker::ArchIntRegStateTracker() {
  reset();
}

const char *ArchIntRegStateTracker::get_name() const {
  return "ArchIntRegState";
}

size_t ArchIntRegStateTracker::get_state_size() const {
  return sizeof(tracker[0]);
}

void ArchIntRegStateTracker::reset() {
  tracker.clear();
}

void ArchIntRegStateTracker::update(DiffTestState *state) {
  tracker.push_back(state->regs.xrf);
}

uint32_t ArchIntRegStateTracker::get_total_states() const {
  return tracker.size();
}

void *ArchIntRegStateTracker::get_state_data(uint32_t i) {
  return (void *)&tracker[i];
}

const void *ArchIntRegStateTracker::get_state_data(uint32_t i) const {
  return (const void *)&tracker[i];
}

void ArchIntRegStateTracker::to_state_bytes(void *bytes) {
  memcpy(bytes, tracker.data(), tracker.size() * get_state_size());
}

#endif // CONFIG_STATE_ARCHINTREG
