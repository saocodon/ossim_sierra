#include <pthread.h>
#include <iostream>
#include <sstream>
#include "memory.hpp"
#include "scheduler.hpp"
#include "cpu.hpp"
#include "configuration.hpp"
#include "loader.hpp"
#include "logger.hpp"

int main(int argc, char** argv) {
  logger::start();

  if (argc < 2) {
    std::cout << "Usage: os [simulation config]\n";
    return 1;
  }

  // reading configuration
  if (configuration::load(argv[1]) == false) {
    std::cout << "Config file unavailable.\n";
    return 1;
  }

  std::cout << "RAM size: "  << configuration::ram_size     << " bytes\n";
  std::cout << "Swap size: " << configuration::swap_size[0] << " bytes\n";
  std::cout << "Cores: "     << configuration::cores        << " cores\n";

  memory*    ram      = new memory(configuration::ram_size    , configuration::page_size);
  memory*    swap     = new memory(configuration::swap_size[0], configuration::page_size);

  scheduler::init();
  loader::init(ram, swap);

  // must be pointers because if they're objects
  // they will automatically be destroyed by C++
  // causing undefined behavior
  cpu* cores[configuration::cores];
  for (int i = 0; i < configuration::cores; i++)
    cores[i] = new cpu(i, configuration::quantum_time);

  for (auto core : cores) core->run();
  for (int i = 0; i < configuration::cores; i++) {
    logger::log("Shutting down CPU " + std::to_string(i) + "...\n");
    cores[i]->halt();
  }

  logger::log("Shutting down logger...\n");

  logger::stop();
  logger::join();

  std::cout << "Cleaning up..." << std::endl;

  delete ram;
  delete swap;
}