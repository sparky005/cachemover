#pragma once

#include "utils/status.h"
#include "utils/stopwatch.h"

#include <memory>
#include <string>
#include <vector>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/sqs/SQSClient.h>

namespace memcachedumper {

class MemoryManager;
class RESTServer;
class Socket;
class SocketPool;
class TaskScheduler;

// Options to configure the cache dumper.
class DumperOptions {
 public:

  void set_memcached_hostname(std::string_view memcached_hostname);
  void set_memcached_port(int memcached_port);
  void set_num_threads(int num_threads);
  void set_chunk_size(uint64_t chunk_size);
  void set_bulk_get_threshold(uint32_t bulk_get_threshold);
  void set_max_memory_limit(uint64_t max_memory_limit);
  void set_max_key_file_size(uint64_t max_key_file_size);
  void set_max_data_file_size(uint64_t max_data_file_size);
  void set_log_file_path(std::string_view logfile_path);
  void set_output_dir_path(std::string_view output_dir_path);
  void set_only_expire_after(int only_expire_after);
  void set_resume_mode(bool resume_mode);
  void set_is_s3_dump(bool is_s3_dump);
  void set_s3_bucket_name(std::string s3_bucket);
  void set_s3_final_path(std::string s3_path);
  void set_req_id(std::string req_id);
  void set_dest_ips_filepath(std::string dest_ips_filepath);
  void set_all_ips_filepath(std::string all_ips_filepath);

  std::string memcached_hostname() { return memcached_hostname_; }
  int memcached_port() { return memcached_port_; }
  int num_threads() { return num_threads_; }
  uint32_t bulk_get_threshold() { return bulk_get_threshold_; }
  uint64_t chunk_size() { return chunk_size_; }
  uint64_t max_memory_limit() { return max_memory_limit_; }
  uint64_t max_key_file_size() { return max_key_file_size_; }
  uint64_t max_data_file_size() { return max_data_file_size_; }
  std::string log_file_path() { return log_file_path_; }
  std::string output_dir_path() { return output_dir_path_; }
  int only_expire_after() { return only_expire_after_; }
  bool is_resume_mode() { return resume_mode_; }
  bool is_s3_dump() { return is_s3_dump_; }
  std::string s3_bucket() { return s3_bucket_; }
  std::string s3_path() { return s3_path_; }
  std::string req_id() { return req_id_; }
  std::string dest_ips_filepath() { return dest_ips_filepath_; }
  std::string all_ips_filepath() { return all_ips_filepath_; }

 private:
  // Hostname that contains target memecached.
  std::string memcached_hostname_;
  // Memcached's port.
  int memcached_port_;
  // Number of worker threads for this cache dumper.
  int num_threads_;
  // Number of keys to bulk get.
  uint32_t bulk_get_threshold_ = 0;
  // The size to use for each allocated buffer (in bytes).
  uint64_t chunk_size_;
  // The maximum amount of memory to use (in bytes).
  uint64_t max_memory_limit_;
  // Max size per key file we produce.
  uint64_t max_key_file_size_;
  // Max size per data file we produce.
  uint64_t max_data_file_size_;
  // Path to the log file.
  std::string log_file_path_;
  // Path to the output directory.
  std::string output_dir_path_;
  // Ignore keys that expire within these many seconds.
  int only_expire_after_ = 0;
  // Indicates that we have to resume from a checkpoint and not start dumping
  // from scratch.
  bool resume_mode_;
  // Uploads to S3 if set.
  bool is_s3_dump_;
  // S3 Bucket name.
  std::string s3_bucket_;
  // S3 Final path.
  std::string s3_path_;
  // Dump request ID.
  std::string req_id_;
  // Path to file containing IP:port pairs of target instances that
  // we want to narrow the dump to.
  std::string dest_ips_filepath_;
  // Path to file containing IP:port pairs of all instances in the target
  // ASG.
  std::string all_ips_filepath_;
};

class Dumper {
 public:
  Dumper(DumperOptions& opts);

  ~Dumper();

  Aws::S3::S3Client* GetS3Client() { return &s3_client_; }
  Aws::SQS::SQSClient* GetSQSClient() { return &sqs_client_; }

  MemoryManager *mem_mgr() { return mem_mgr_.get(); }

  std::string TimeElapsed() { return total_msw_.HumanElapsedStr(); }

  // Initializes the dumper by connecting to memcached.
  Status Init();

  // Initialize the SQS queue.
  Status InitSQS();

  // Starts the dumping process.
  void Run();

  // Returns a socket to memcached from the socket pool.
  Socket* GetMemcachedSocket();

  // Releases a memcached socket back to the socket pool.
  void ReleaseMemcachedSocket(Socket *sock);

 private:

  // Set up the output directories and make sure they're empty.
  Status CreateAndValidateOutputDirs();

  std::string memcached_hostname_;

  DumperOptions opts_;

  // Pool of sockets to talk to memcached.
  std::unique_ptr<SocketPool> socket_pool_;

  // Owned memory manager.
  std::unique_ptr<MemoryManager> mem_mgr_;

  // The task scheduler that will carry out all the work.
  std::unique_ptr<TaskScheduler> task_scheduler_;

  // A REST Server to report metrics.
  std::unique_ptr<RESTServer> rest_server_;

  Aws::Client::ClientConfiguration s3_config_;
  Aws::S3::S3Client s3_client_;
  Aws::SQS::SQSClient sqs_client_;

  // Select IP addresses that we want to dump to. To use, 'all_ips_' must be provided
  // and 'dest_ips_' must be a subset of 'all_ips_'. Optional.
  std::vector<std::string> dest_ips_;
  // All IP addresses of target ASG. To use, 'dest_ips_' must be provided and it
  // must be a subset of 'all_ips_'. Optional.
  std::vector<std::string> all_ips_;

  // Stopwatch to run for the entire duration of the dumper.
  MonotonicStopWatch total_msw_;

};

} // namespace memcachedumper
