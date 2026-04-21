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

#ifndef __STATE_H
#define __STATE_H

#include "common.h"
#include <cstddef>
#include <vector>

class StateTracker{
public:
  StateTracker() = default;
  virtual ~StateTracker() = default;

  virtual const char *get_name() const = 0;
  virtual size_t get_state_size() const = 0;
  virtual void reset() = 0;
  virtual void update(DiffTestState *state) = 0;

  // tracker figures
  virtual uint32_t get_total_states() const = 0;
  virtual void* get_state_data(uint32_t i) = 0;
  virtual const void* get_state_data(uint32_t i) const = 0;

  // fuzzer feedback
  bool is_feedback = false;
  virtual void update_is_feedback(const char *state_name) {
    is_feedback = !state_name_cmp(state_name, get_name());
  }
  virtual void to_state_bytes(void *bytes) = 0;

protected:
  static int state_name_cmp(const char *s1, const char *s2) {
    for (int i = 0; s1[i] || s2[i]; i++) {
      char a = (s1[i] >= 'A' && s1[i] <= 'Z') ? s1[i] + ('a' - 'A') : s1[i];
      char b = (s2[i] >= 'A' && s2[i] <= 'Z') ? s2[i] + ('a' - 'A') : s2[i];
      if (a != b) {
        return i + 1;
      }
    }
    return 0;
  }
};

#if defined(CONFIG_DIFFTEST_ARCHINTREGSTATE) && defined(CONFIG_DIFFTEST_INSTRCOMMIT)
#define CONFIG_STATE_ARCHINTREG
class ArchIntRegStateTracker : public StateTracker {
public:
  ArchIntRegStateTracker();
  ~ArchIntRegStateTracker() = default;

  const char *get_name() const final;
  size_t get_state_size() const final;
  void reset() final;
  void update(DiffTestState *state) final;

  uint32_t get_total_states() const final;
  void* get_state_data(uint32_t i) final;
  const void* get_state_data(uint32_t i) const final;

  void to_state_bytes(void *bytes) final;

  // const char *get_name() const {
  //   return "ArchIntReg";
  // }

  // void reset() {
  //   sequence.clear();
  // }

  // void update(DiffTestState *state) {
  //   sequence.push_back(state->regs_int);
  // }

  // size_t get_entry_count() const {
  //   return sequence.size();
  // }

  // void to_state_bytes(uint8_t *bytes) {
  //   memcpy(bytes, sequence.data(), sequence.size() * sizeof(sequence[0]));
  // };

private:
  std::vector<DifftestArchIntRegState> tracker;
};
#endif // CONFIG_DIFFTEST_ARCHINTREGSTATE && CONFIG_DIFFTEST_INSTRCOMMIT

#endif // __STATE_H
