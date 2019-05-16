#include <string_view>
#include <vector>
#include <utility>

struct Event {
  Event(const char* n, double t): name(n), elapsed_time(t) {}
  const char* name = "";
  double elapsed_time = 0;
};

struct Frame {
  double start_time = 0;
  double end_time = 0; // end time of last event until endFrame() is called, then actual frame's end time
  std::vector<Event> _events;
  double elapsedTime() const { return end_time - start_time; }
};

struct Profiler {
  void startFrame();
  void event(const char* name);
  void endFrame();
  void removeLastFrame();
  std::vector<Frame> _frames;
};