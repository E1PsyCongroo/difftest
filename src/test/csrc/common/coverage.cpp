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

#include "coverage.h"
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <unistd.h>

#if VM_COVERAGE == 1
#include "VSimTop.h"
#include "verilated.h"
#include "verilated_cov.h"
#endif // VM_COVERAGE

void Coverage::display_uncovered_points() {
  printf("Uncovered %s coverage points:\n", get_name());
  for (auto i = 0; i < get_total_points(); i++) {
    if (!is_accumulated(i)) {
      printf("  [%d] %s\n", i, get_cover_name(i));
    }
  }
}

#ifdef FIRRTL_COVER
FIRRTLCoverage::FIRRTLCoverage() {
  for (int i = 0; i < n_cover; i++) {
    acc[i] = new uint8_t[firrtl_cover[i].cover.total];
  }
};

FIRRTLCoverage::~FIRRTLCoverage() {
  for (int i = 0; i < n_cover; i++) {
    delete acc[i];
  }
}

const char *FIRRTLCoverage::get_cover_name(uint32_t i) {
  if (i >= get_total_points()) {
    return nullptr;
  }
  return get()->point_names[i];
}

void FIRRTLCoverage::reset() {
  for (auto c: firrtl_cover) {
    memset(c.cover.points, 0, c.cover.total);
  }
}

uint32_t FIRRTLCoverage::get_total_points() {
  return get()->total;
}

uint32_t FIRRTLCoverage::get_covered_points() {
  return cover_sum(get());
}

void FIRRTLCoverage::accumulate() {
  for (int i = 0; i < n_cover; i++) {
    for (auto j = 0; j < firrtl_cover[i].cover.total; j++) {
      if (firrtl_cover[i].cover.points[j]) {
        acc[i][j] = 1;
      }
    }
  }
}

uint32_t FIRRTLCoverage::get_acc_covered_points() {
  auto target = get();
  auto i = (FIRRTLCoverPointParam *)target - firrtl_cover;
  return cover_sum(acc[i], firrtl_cover[i].cover.total);
}

void FIRRTLCoverage::display() {
  for (int i = 0; i < n_cover; i++) {
    display(i);
  }
}

void FIRRTLCoverage::display(int i) {
  uint32_t covered = cover_sum(&(firrtl_cover[i].cover));
  uint32_t acc_value = cover_sum(acc[i], firrtl_cover[i].cover.total);
  Coverage::display(firrtl_cover[i].cover.name, firrtl_cover[i].cover.total, covered, acc_value);
}

void FIRRTLCoverage::display_uncovered_points() {
  for (int i = 0; i < n_cover; i++) {
    printf("Uncovered %s coverage points:\n", firrtl_cover[i].cover.name);
    for (auto j = 0; j < firrtl_cover[i].cover.total; j++) {
      if (!acc[i][j]) {
        printf("  [%d] %s\n", j, firrtl_cover[i].cover.point_names[j]);
      }
    }
  }
}

void FIRRTLCoverage::update_is_feedback(const char *cover_name) {
  // cover_name should be get_name().firrtl_cover_name
  auto name_len = strlen(get_name());
  auto cmp = cover_name_cmp(cover_name, get_name());
  is_feedback = cmp > name_len || !cmp;
  if (is_feedback && cover_name[name_len]) {
    // skip the name and dot (.)
    auto found = false;
    auto subname = cover_name + name_len + 1;
    for (auto &c: firrtl_cover) {
      c.is_feedback = !cover_name_cmp(subname, c.cover.name);
      found = found || c.is_feedback;
    }
    if (!found) {
      printf("Unknown subtype of FIRRTLCoverage: %s\n", cover_name);
      assert(0);
    }
  }
}

size_t FIRRTLCoverage::cover_data_size() {
  return sizeof(target->points[0]);
}

void FIRRTLCoverage::to_cover_data(void *data) {
  auto target = get();
  memcpy(bytes, target->points, target->total);
}

const FIRRTLCoverPoint *FIRRTLCoverage::get() {
  for (auto &c: firrtl_cover) {
    if (c.is_feedback) {
      return &(c.cover);
    }
  }
  return nullptr;
}

uint32_t FIRRTLCoverage::cover_sum(uint8_t *points, uint32_t total) {
  uint32_t result = 0;
  for (int i = 0; i < total; i++) {
    result += points[i];
  }
  return result;
}

uint32_t FIRRTLCoverage::cover_sum(const FIRRTLCoverPoint *cover) {
  return cover_sum(cover->points, cover->total);
}
#endif // FIRRTL_COVER

#if VM_COVERAGE == 1
static void verilator_append_point_field(std::string &result, const char *key, const std::string &value) {
  if (value.empty()) {
    return;
  }
  if (!result.empty()) {
    result += ", ";
  }
  result += key;
  result += ": ";
  result += value;
}

static std::string verilator_format_point_name(const std::string &file, const std::string &line,
                                               const std::string &col) {
  std::string result;
  verilator_append_point_field(result, "f", file);
  verilator_append_point_field(result, "l", line);
  verilator_append_point_field(result, "n", col);
  return result;
}

static bool verilator_parse_field(const std::string &field, std::string &key, std::string &value) {
  auto sep = field.find('\002');
  if (sep == std::string::npos) {
    return false;
  }
  key = field.substr(0, sep);
  value = field.substr(sep + 1);
  return true;
}

static bool verilator_parse_line(const std::string &line, std::string &type, std::string &point_name, uint64_t &count) {
  if (line.rfind("C '", 0) != 0) {
    return false;
  }

  auto metadata_begin = line.find('\'');
  if (metadata_begin == std::string::npos) {
    return false;
  }
  auto metadata_end = line.find('\'', metadata_begin + 1);
  if (metadata_end == std::string::npos) {
    return false;
  }

  auto metadata = line.substr(metadata_begin + 1, metadata_end - metadata_begin - 1);
  auto count_str = line.substr(metadata_end + 1);
  try {
    count = std::stoull(count_str);
  } catch (...) {
    return false;
  }

  size_t pos = 0;
  std::string file, lineno, column;
  while (pos < metadata.size()) {
    auto next = metadata.find('\001', pos);
    auto field = metadata.substr(pos, next == std::string::npos ? next : next - pos);
    std::string key, value;
    if (verilator_parse_field(field, key, value)) {
      if (key == "t") {
        type = value;
      } else if (key == "f") {
        file = value;
      } else if (key == "l") {
        lineno = value;
      } else if (key == "n") {
        column = value;
      }
    }
    if (next == std::string::npos) {
      break;
    }
    pos = next + 1;
  }

  if (type.empty()) {
    return false;
  }
  point_name = verilator_format_point_name(file, lineno, column);
  return true;
}

VerilatorCoverage::VerilatorCoverage()
    : cover{VerilatorCoverGroup("line"), VerilatorCoverGroup("branch"), VerilatorCoverGroup("expr")} {
  auto context = new VerilatedContext;
  Verilated::threadContextp(context);
  auto model = new VSimTop(context);
  load_from_verilator();
  delete model;
  Verilated::threadContextp(nullptr);
  delete context;
}

const char *VerilatorCoverage::get_cover_name(uint32_t i) {
  if (auto group = get(); group && i < group->points.size()) {
    return group->points[i].name.c_str();
  }
  return nullptr;
}

void VerilatorCoverage::reset() {
  for (auto &group: cover) {
    for (auto &point: group.points) {
      point.count = 0;
    }
  }
  Verilated::threadContextp()->coveragep()->clear();
}

void VerilatorCoverage::update(const DiffTestState *state) {
  load_from_verilator();
}

uint32_t VerilatorCoverage::get_total_points() {
  if (auto group = get()) {
    return group->points.size();
  }
  return 0;
}

uint32_t VerilatorCoverage::get_covered_points() {
  return cover_sum(false);
}

void VerilatorCoverage::accumulate() {
  for (auto &group: cover) {
    for (auto &point: group.points) {
      if (point.count) {
        point.acc = true;
      }
    }
  }
}

bool VerilatorCoverage::is_accumulated(uint32_t i) {
  if (auto group = get(); group && i < group->points.size()) {
    return group->points[i].acc;
  }
  return false;
}

uint32_t VerilatorCoverage::get_acc_covered_points() {
  return cover_sum(true);
}

void VerilatorCoverage::display() {
  for (const auto &group: cover) {
    display(group);
  }
}

void VerilatorCoverage::display_uncovered_points() {
  for (const auto &group: cover) {
    printf("Uncovered %s coverage points:\n", group.name.c_str());
    for (uint32_t i = 0; i < group.points.size(); i++) {
      if (!group.points[i].acc) {
        printf("  [%d] %s\n", i, group.points[i].name.c_str());
      }
    }
  }
}

void VerilatorCoverage::update_is_feedback(const char *cover_name) {
  auto name_len = strlen(get_name());
  auto cmp = cover_name_cmp(cover_name, get_name());
  is_feedback = cmp > name_len || !cmp;
  feedback_group = -1;
  if (is_feedback && cover_name[name_len]) {
    // skip the name and dot (.)
    auto found = false;
    auto subname = cover_name + name_len + 1;
    for (int i = 0; i < cover.size(); ++i) {
      if (!cover_name_cmp(subname, cover[i].name.c_str())) {
        feedback_group = i;
        found = true;
      }
    }
    Assert(found, "Unknown subtype of VerilatorCoverage: %s\n", cover_name);
  }
}

size_t VerilatorCoverage::cover_data_size() {
  return sizeof(uint64_t);
}

void VerilatorCoverage::to_cover_data(void *data) {
  if (auto group = get()) {
    for (size_t i = 0; i < group->points.size(); ++i) {
      ((uint64_t *)data)[i] = group->points[i].count;
    }

    // printf("[DIFFTEST DEBUG]: cover points of %s:\n", group->name.c_str());
    // for (uint32_t i = 0; i < group->points.size(); ++i) {
    //   printf("[%d]: %zd\n", i, group->points[i].count);
    // }
    // fflush(stdout);

  }
}

void VerilatorCoverage::load_from_verilator() {
  char filename[] = "/tmp/verilator-coverage-XXXXXX";
  auto fd = mkstemp(filename);
  Assert(fd >= 0, "Failed to create temporary file for Verilator coverage");
  close(fd);

  Verilated::threadContextp()->coveragep()->write(filename);
  load_from_file(filename);
  unlink(filename);
}

void VerilatorCoverage::load_from_file(const char *filename) {
  std::vector<VerilatorCoverPoint> line;
  std::vector<VerilatorCoverPoint> branch;
  std::vector<VerilatorCoverPoint> expr;

  std::ifstream input(filename);
  std::string text;
  while (std::getline(input, text)) {
    std::string type{}, point_name{};
    uint64_t count{0};
    if (!verilator_parse_line(text, type, point_name, count)) {
      continue;
    }

    if (type == "line") {
      line.emplace_back(point_name, count);
    }
    else if (type == "expr") {
      expr.emplace_back(point_name, count);
    }
    else if (type == "branch") {
      branch.emplace_back(point_name, count);
    } else {
      printf("Unknown coverage type from Verilator: %s\n", type.c_str());
      assert(0);
    }
  }

  update_group("line", line);
  update_group("branch", branch);
  update_group("expr", expr);
}

void VerilatorCoverage::update_group(const std::string &type, std::vector<VerilatorCoverPoint> &points) {
  auto group = get(type);
  assert(group);

  if (!group->points.empty()) {
    assert(group->points.size() == points.size());
    for (size_t i = 0; i < points.size(); i++) {
      Assert(group->points[i].name == points[i].name,
             "Point name mismatch for Verilator coverage group %s at index %zu: %s vs %s\n", type.c_str(), i,
             group->points[i].name.c_str(), points[i].name.c_str());
      points[i].acc = group->points[i].acc;
    }
  }

  group->points = std::move(points);
}

VerilatorCoverGroup *VerilatorCoverage::get() {
  if (feedback_group < 0) {
    return nullptr;
  }
  return &cover[feedback_group];
}

VerilatorCoverGroup *VerilatorCoverage::get(const std::string &type) {
  for (auto &group: cover) {
    if (group.name == type) {
      return &group;
    }
  }
  return nullptr;
}

uint32_t VerilatorCoverage::cover_sum(const VerilatorCoverGroup &group, bool accumulated) {
  uint32_t result = 0;
  for (const auto &point: group.points) {
    result += accumulated ? point.acc : point.count != 0;
  }
  return result;
}

uint32_t VerilatorCoverage::cover_sum(bool accumulated) {
  if (auto group = get()) {
    return cover_sum(*group, accumulated);
  }
  return 0;
}

void VerilatorCoverage::display(const VerilatorCoverGroup &group) {
  Coverage::display(("verilator." + group.name).c_str(), group.points.size(), cover_sum(group, false), cover_sum(group, true));
  // printf("%s coverage points:\n", group.name.c_str());
  // for (uint32_t i = 0; i < group.points.size(); i++) {
  //   printf("  [%d] %s: %zu\n", i, group.points[i].name.c_str(), group.points[i].count);
  // }
}

#endif // VM_COVERAGE

#ifdef LLVM_COVER
typedef struct {
  void *pc;
  uint64_t tag;
} llvm_sancov_pc_t;

LLVMSanCovData *llvm_sancov = nullptr;

extern "C" void __sanitizer_cov_trace_pc_guard_init(uint32_t *start, uint32_t *stop) {
  static uint32_t count = 0;
  if (start == stop || *start)
    return;
  if (!llvm_sancov) {
    llvm_sancov = new LLVMSanCovData();
  }
  auto n_cover = stop - start;
  auto s = llvm_sancov->points.size();
  llvm_sancov->points.resize(llvm_sancov->points.size() + n_cover, false);
  for (uint32_t *x = start; x < stop; x++) {
    *x = ++count;
  }
}

extern "C" void __sanitizer_symbolize_pc(uintptr_t pc, const char *fmt, char *out, size_t out_size);
extern "C" void __sanitizer_cov_trace_pc_guard(uint32_t *guard) {
  if (!*guard)
    return;
  auto index = *guard;
  if (llvm_sancov->points.size() >= index) {
    llvm_sancov->points[index - 1] = true;
    llvm_sancov->reach++;
  }
  *guard = 0;
}

extern "C" void __sanitizer_cov_pcs_init(const uintptr_t *pcs_beg, const uintptr_t *pcs_end) {
  char info_str[1024] = "(?) ";
  char *pcDescr = info_str + 4;
  auto p = (const llvm_sancov_pc_t *)pcs_beg;
  auto n = (const llvm_sancov_pc_t *)pcs_end - p;
  for (int i = 0; i < n; i++, p++) {
    info_str[1] = p->tag ? 'Y' : 'N';
    auto pc = (uintptr_t)p->pc + (p->tag ? 1 : 0);
    __sanitizer_symbolize_pc(pc, "%p %F %L", pcDescr, sizeof(info_str) - 4);
    std::string str(info_str);
    llvm_sancov->info.push_back(str);
  }
}
#endif // LLVM_COVER

UnionCoverage::UnionCoverage(Coverage *_c1, Coverage *_c2) : c1(_c1), c2(_c2) {}

void UnionCoverage::reset() {
  c1->reset();
  c2->reset();
}

uint32_t UnionCoverage::get_total_points() {
  return c1->get_total_points() + c2->get_total_points();
}

uint32_t UnionCoverage::get_covered_points() {
  return c1->get_covered_points() + c2->get_covered_points();
}

void UnionCoverage::accumulate() {
  c1->accumulate();
  c2->accumulate();
}

uint32_t UnionCoverage::get_acc_covered_points() {
  return c1->get_acc_covered_points() + c2->get_acc_covered_points();
}

void UnionCoverage::display_uncovered_points() {
  c1->display_uncovered_points();
  c2->display_uncovered_points();
}

// cover_name should be get_name():c1->get_name()+c1->get_name()
void UnionCoverage::update_is_feedback(const char *cover_name) {
  auto name_len = strlen(get_name()) + strlen(c1->get_name()) + strlen(c2->get_name()) + 2;
  char *correct_name = new char[name_len + 1];
  snprintf(correct_name, name_len + 1, "%s:%s+%s", get_name(), c1->get_name(), c2->get_name());
  is_feedback = !cover_name_cmp(cover_name, correct_name);
  delete[] correct_name;
}

size_t UnionCoverage::cover_data_size() {
  return c1->cover_data_size() + c2->cover_data_size();
}

void UnionCoverage::to_cover_data(void *data) {
  c1->to_cover_data(data);
  c2->to_cover_data((uint8_t *)data + c1->get_total_points() * c1->cover_data_size());
}
