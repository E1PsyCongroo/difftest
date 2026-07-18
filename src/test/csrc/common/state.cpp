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
#include "difftest-state.h"
#include "refproxy.h"
#include <cstdint>

PCStateTracker::PCStateTracker() : tracker{} {}

void PCStateTracker::reset() {
  tracker.clear();
}

void PCStateTracker::update(const DiffTestState *state) {
  tracker.push_back(state->commit->pc);
}

uint64_t PCStateTracker::get_total_states() {
  return tracker.size();
}

void *PCStateTracker::get_state_data(uint64_t i) {
  return &tracker[i];
}

void PCStateTracker::to_state_bytes(void *bytes) {
  memcpy(bytes, tracker.data(), tracker.size() * sizeof(uint64_t));
}

ArchIntRegStateTracker::ArchIntRegStateTracker() : tracker{} {}

void ArchIntRegStateTracker::reset() {
  tracker.clear();
}

void ArchIntRegStateTracker::update(const DiffTestState *state) {
  tracker.push_back(state->regs.xrf);
}

uint64_t ArchIntRegStateTracker::get_total_states() {
  return tracker.size();
}

void *ArchIntRegStateTracker::get_state_data(uint64_t i) {
  return &tracker[i];
}

void ArchIntRegStateTracker::to_state_bytes(void *bytes) {
  memcpy(bytes, tracker.data(), tracker.size() * sizeof(DifftestArchIntRegState));
}

CSRStateTracker::CSRStateTracker() {
  reset();
}

void CSRStateTracker::reset() {
  tracker.clear();
}

void CSRStateTracker::update(const DiffTestState *state) {
  tracker.push_back(state->regs.csr);
}

uint64_t CSRStateTracker::get_total_states() {
  return tracker.size();
}

void *CSRStateTracker::get_state_data(uint64_t i) {
  return &tracker[i];
}

void CSRStateTracker::to_state_bytes(void *bytes) {
  memcpy(bytes, tracker.data(), tracker.size() * sizeof(DifftestCSRState));
}
