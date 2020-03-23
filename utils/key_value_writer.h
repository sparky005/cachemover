#pragma once

#include "utils/memcache_utils.h"

#include <string>
#include <unordered_map>

namespace memcachedumper {

// Forward declarations.
class Socket;

class KeyValueWriter {
 public:
  KeyValueWriter(uint8_t* buffer, size_t capacity, Socket* mc_sock);

  // Adds 'mc_key' to the entries to get the value for and write to a file.
  void ProcessKey(McData* mc_key);

  // Flushes pending keys for bulk get and writing, if any, from the mcdata_entries_.
  void FlushPending();

  void PrintKeys();
 private:

  // Get values from Memcached for keys present in mcdata_entries_ map.
  void BulkGetKeys();

  // Process the response from Memcached and populate mcdata_entries_ appropriately.
  // Returns 'true' if the entire response was processed, i.e. if "END\r\n" was seen
  // in 'buffer'.
  bool ProcessBulkResponse(uint8_t* buffer, int32_t bufsize);

  // Writes entires marked as complete from 'mcdata_entries_' to the final output
  // file.
  void WriteCompletedEntries(uint32_t num_complete_entries);

  // The buffer to use while getting values from Memcached.
  uint8_t* buffer_;
  // The size of 'buffer_'.
  size_t capacity_;
  // Socket to talk to Memcached.
  Socket* mc_sock_;
  // Map of key name to McData entries.
  std::unordered_map<std::string, std::unique_ptr<McData>> mcdata_entries_;

  // While parsing responses from Memcached, this keeps track of the current key
  // being processed.
  // We keep track of this since the response for a key can be truncated at the end of
  // 'buffer_', and we need to continue processing the same key after obtaining relevant
  // information from the next batch of bytes from Memcached.
  std::string cur_key_;

  // This is the last state seen in a buffer being processed. It's used to find out where
  // our buffer got truncated, so we can continue from there.
  DataBufferSlice::ResponseFormatState broken_buffer_state_;

  // Was the last buffer processed a partial buffer?
  bool last_buffer_partial_;

};

} // namespace memcachedumper
