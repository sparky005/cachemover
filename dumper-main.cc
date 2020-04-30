#include "common/logger.h"
#include "dumper/dumper.h"
#include "utils/status.h"

#include <iostream>
#include <string>

using std::string;
namespace memcachedumper {

void DumperMain(DumperOptions& opts) {

  // Initialize our global logger.
  Logger::InitGlobalLogger("global_logger", opts.logfile_path());

  // Set up the dumper with the provided options.
  Dumper dumper(opts);
  Status dumper_status = dumper.Init();
  if (!dumper_status.ok()) {
    LOG_ERROR("Received fatal error: " + dumper_status.ToString());
    exit(-1);
  }

  // Begin dumping.
  dumper.Run();
}

} // namespace memcachedumper


int main(int argc, char** argv) {

  // TODO: Take command line arguments for the DumperOptions.

  memcachedumper::DumperOptions dummy_options;
  dummy_options.set_hostname("127.0.0.1");
  dummy_options.set_port(11211);
  dummy_options.set_num_threads(16);
  dummy_options.set_chunk_size(67108864); // 1MB
  dummy_options.set_max_memory_limit(2147483648); // 64MB
  dummy_options.set_max_key_file_size(268435456); // 1MB
  dummy_options.set_max_data_file_size(1073741824); // 1MB
  dummy_options.set_logfile_path("logfile.txt");

  memcachedumper::DumperMain(dummy_options);

  LOG("Exiting program!");
  return 0;
}

