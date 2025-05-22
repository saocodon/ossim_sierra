#include "memory.hpp"

memory::memory(int max_size, int page_size)
{
  storage = new char[max_size];
  memset(storage, 0, max_size);

  this->max_size = max_size;
  this->page_size = page_size;

  this->frame_count = max_size / page_size;
  for (int i = 0; i < frame_count; i++)
    free_frames.push_back(i);
}

int memory::get_free_frame()
{
  if (free_frames.empty()) return -1;
  int frame = free_frames.front();
  free_frames.pop_front();
  used_frames.push_back(frame);
  return frame;
}