#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

class logger
{
private:
  static std::queue<std::string> messages;
  static std::thread             t;
  static std::mutex              mtx;
  static std::condition_variable cv;
  static bool                    done;

  static void run();

public:
  static void start();
  static void log(const std::string message);
  static void stop();
  static void join();
};

#endif
