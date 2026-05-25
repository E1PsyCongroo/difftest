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
#include <cstdint>
#include <vector>

class StateTracker{
public:
  StateTracker() = default;
  virtual ~StateTracker() = default;

  virtual const char *get_name() const = 0;
  virtual void reset() = 0;
  virtual void update(const DiffTestState *state) = 0;

  // tracker figures
  virtual uint64_t get_total_states() = 0;
  virtual void* get_state_data(uint64_t i) = 0;

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

class PCStateTracker : public StateTracker {
public:
  PCStateTracker();
  ~PCStateTracker() = default;

  const char *get_name() const {
    return "PCState";
  }
  void reset() final;
  void update(const DiffTestState *state) final;

  uint64_t get_total_states() final;
  void* get_state_data(uint64_t i) final;

  void to_state_bytes(void *bytes) final;
private:
  std::vector<uint64_t> tracker;
};

class ArchIntRegStateTracker : public StateTracker {
public:
  ArchIntRegStateTracker();
  ~ArchIntRegStateTracker() = default;

  const char *get_name() const {
    return "ArchIntRegState";
  }
  void reset() final;
  void update(const DiffTestState *state) final;

  uint64_t get_total_states() final;
  void* get_state_data(uint64_t i) final;

  void to_state_bytes(void *bytes) final;
private:
  std::vector<DifftestArchIntRegState> tracker;
};

class CSRStateTracker : public StateTracker {
public:
  CSRStateTracker();
  ~CSRStateTracker() = default;

  const char *get_name() const {
    return "CSRState";
  }
  void reset() final;
  void update(const DiffTestState *state) final;

  uint64_t get_total_states() final;
  void* get_state_data(uint64_t i) final;

  void to_state_bytes(void *bytes) final;
private:
  std::vector<DifftestCSRState> tracker;
};


#endif // __STATE_H
