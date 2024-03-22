/*
 * Copyright © 2020-2021 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <pps/pps_driver.h>

#include <charconv>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <thread>

#include <docopt/docopt.h>

static const char *USAGE =
   R"(pps-config

  Usage:
	pps-config info
	pps-config dump [--gpu=<n>] [--ids=<n>] [--sec=<n>]
	pps-config groups [--gpu=<n>]
	pps-config counters [--gpu=<n>]
	pps-config (-h | --help)
	pps-config --version

  Options:
	-h --help  Show this screen.
	--version  Show version.
	--gpu=<n>  GPU number to query [default: 0].
	--ids=<n>  Comma separated list of numbers.
	--sec=<n>  Seconds to wait before dumping performance counters [default: 1].
)";

// Tool running mode
enum class Mode {
   // Show help message
   Help,

   // Show system information
   Info,

   // Show list of available counters
   Counters,

   // Groups
   Groups,

   // Dump performance counters
   Dump,
};

std::vector<std::string_view> split(const std::string &list, const std::string &separator)
{
   std::vector<std::string_view> ret;
   std::string_view list_view = list;
   while (!list_view.empty()) {
      size_t pos = list_view.find(separator);
      if (pos == std::string::npos) {
         ret.push_back(list_view);
         break;
      }
      ret.push_back(list_view.substr(0, pos));
      list_view = list_view.substr(pos + separator.length(), list_view.length());
   }
   return ret;
}

std::optional<uint32_t> to_counter_id(const std::string_view &view)
{
   uint32_t counter_id = 0;

   auto res = std::from_chars(view.data(), view.data() + view.size(), counter_id);
   if (res.ec == std::errc::invalid_argument) {
      return std::nullopt;
   }

   return counter_id;
}

int main(int argc, const char **argv)
{
   using namespace pps;

   Mode mode = Mode::Help;
   auto secs = std::chrono::seconds(1);
   uint32_t gpu_num = 0;
   std::vector<uint32_t> counter_ids;

   auto args =
      docopt::docopt(USAGE, {std::next(argv), std::next(argv, argc)}, true, "pps-config 0.3");

   if (args["info"].asBool()) {
      mode = Mode::Info;
   }

   if (args["dump"].asBool()) {
      mode = Mode::Dump;
   }

   if (args["--gpu"]) {
      gpu_num = static_cast<uint32_t>(args["--gpu"].asLong());
   }

   if (args["--ids"]) {
      auto comma_separated_list = args["--ids"].asString();
      std::vector<std::string_view> ids_list = split(comma_separated_list, ",");

      for (auto &id : ids_list) {
         if (auto counter_id = to_counter_id(id)) {
            counter_ids.push_back(*counter_id);
         } else {
            fprintf(stderr, "Failed to parse counter ids: %s\n", comma_separated_list.c_str());
            return EXIT_FAILURE;
         }
      }
   }

   if (args["--sec"]) {
      secs = std::chrono::seconds(args["--sec"].asLong());
   }

   if (args["groups"].asBool()) {
      mode = Mode::Groups;
   }

   if (args["counters"].asBool()) {
      mode = Mode::Counters;
   }

   // Docopt shows the help message for us
   if (mode == Mode::Help) {
      return EXIT_SUCCESS;
   }

   switch (mode) {
   default:
      break;
   case Mode::Info: {
      // Header: device name, and whether it is supported or not
      printf("#%4s %16s %16s\n", "num", "device", "support");

      auto devices = DrmDevice::create_all();
      for (auto &device : devices) {
         auto gpu_num = device.gpu_num;
         auto name = device.name;
         auto driver = Driver::get_driver(std::move(device));
         printf(" %4u %16s %16s\n", gpu_num, name.c_str(), driver ? "yes" : "no");
      }

      break;
   }
   case Mode::Dump: {
      if (auto device = DrmDevice::create(gpu_num)) {
         if (auto driver = Driver::get_driver(std::move(device.value()))) {
            driver->init_perfcnt();

            // Enable counters
            if (counter_ids.empty()) {
               driver->enable_all_counters();
            } else {
               for (auto id : counter_ids) {
                  driver->enable_counter(id);
               }
            }

            driver->enable_perfcnt(std::chrono::nanoseconds(secs).count());
            std::this_thread::sleep_for(std::chrono::seconds(secs));

            // Try dumping until it succeeds
            while (!driver->dump_perfcnt())
               ;
            // Try collecting samples until it succeeds
            while (!driver->next())
               ;

            printf("#%32s %32s\n", "counter", "value");
            for (auto &counter : driver->enabled_counters) {
               printf(" %32s ", counter.name.c_str());
               auto value = counter.get_value(*driver);
               if (auto d_val = std::get_if<double>(&value)) {
                  printf("%32f\n", *d_val);
               } else if (auto i_val = std::get_if<int64_t>(&value))
                  printf("%32li\n", *i_val);
               else {
                  printf("%32s\n", "error");
               }
            }
         }
      }
      break;
   }
   case Mode::Groups: {
      if (auto device = DrmDevice::create(gpu_num)) {
         if (auto driver = Driver::get_driver(std::move(device.value()))) {
            driver->init_perfcnt();
            printf("#%4s %32s\n", "id", "name");

            for (auto &group : driver->groups) {
               printf(" %4u %32s\n", group.id, group.name.c_str());
            }
         }
      }

      break;
   }
   case Mode::Counters: {
      if (auto device = DrmDevice::create(gpu_num)) {
         if (auto driver = Driver::get_driver(std::move(device.value()))) {
            driver->init_perfcnt();
            printf("#%4s %32s\n", "id", "name");

            for (uint32_t i = 0; i < driver->counters.size(); ++i) {
               auto &counter = driver->counters[i];
               printf(" %4u %32s\n", counter.id, counter.name.c_str());
            }
         }
      }

      break;
   }
   } // switch

   return EXIT_SUCCESS;
}
