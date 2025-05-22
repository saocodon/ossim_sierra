#include "logger.hpp"

// Static members initialization
std::queue<std::string> logger::messages;
std::thread             logger::t;
std::mutex              logger::mtx;
std::condition_variable logger::cv;
bool                    logger::done = false;

void logger::run()
{
  std::unique_lock<std::mutex> lock(mtx);
  while (!done)
  {
    cv.wait(lock, [] { return !messages.empty() || done; });
    while (!messages.empty())
    {
      std::cout << messages.front();
      messages.pop();
    }
  }
}

void logger::start()
{
  done = false;
  t = std::thread(run);
}

void logger::log(const std::string message)
{
  {
    std::lock_guard<std::mutex> lock(mtx);
    messages.push(message);
  }
  cv.notify_one();
}

void logger::stop()
{
  {
    std::lock_guard<std::mutex> lock(mtx);
    done = true;
  }
  cv.notify_all();
}

void logger::join()
{
  if (t.joinable())
    t.join();
}
