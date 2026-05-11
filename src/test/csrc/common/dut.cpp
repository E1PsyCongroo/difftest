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

#include "dut.h"
#include <cstdint>
#ifndef CONFIG_NO_DIFFTEST
#include "difftest.h"
#endif

SimStats stats;

// Coverage
extern "C" uint32_t get_cover_number() {
  if (auto c = stats.get_feedback_cover()) {
    return c->get_total_points();
  }
  return 0;
}

extern "C" const char *get_cover_point_name(size_t i) {
  if (auto c = stats.get_feedback_cover()) {
    return c->get_cover_name(i);
  }
  return nullptr;
}

extern "C" size_t get_cover_data_size() {
  if (auto c = stats.get_feedback_cover()) {
    return c->cover_data_size();
  }
  return 0;
}

extern "C" void update_stats_cover(void *icover_data) {
  if (auto c = stats.get_feedback_cover()) {
    c->to_cover_data(icover_data);
  }
}

extern "C" void display_uncovered_points() {
  stats.display_uncovered_points();
}

extern "C" void set_cover_feedback(const char *name) {
  stats.set_feedback_cover(name);
}

// StateSequence
extern "C" size_t get_state_number() {
  if (auto s = stats.get_feedback_state()) {
    return s->get_total_states();
  }
  return 0;
}

extern "C" size_t get_state_size() {
  if (auto s = stats.get_feedback_state()) {
    return s->get_state_size();
  }
  return 0;
}

extern "C" void update_stats_state(void *state_tracker) {
  if (auto c = stats.get_feedback_state()) {
    c->to_state_bytes(state_tracker);
  }
}

extern "C" void set_state_feedback(const char *name) {
  stats.set_feedback_state(name);
}

// PC Trace
extern "C" size_t get_pc_trace_size() {
  return stats.pc_trace.size();
}

extern "C" void update_stats_pc_trace(uint64_t *pc_trace) {
  memcpy(pc_trace, stats.pc_trace.data(), sizeof(uint64_t) * stats.pc_trace.size());
}
