#include "Profiler.h"
#include <GLFW/glfw3.h>

void Profiler::startFrame() {
  _frames.emplace_back();
  _frames.back().start_time = glfwGetTime();
}

void Profiler::event(const char* name) {
  auto& curr = _frames.back();
  auto event_start_time = curr._events.empty() ? curr.start_time : curr.end_time;
  curr.end_time = glfwGetTime();
  auto elapsed_time = curr.end_time - event_start_time;
  curr._events.emplace_back(name, elapsed_time);
}

void Profiler::endFrame() {
  _frames.back().end_time = glfwGetTime();
}

void Profiler::removeLastFrame() {
  _frames.pop_back();
}