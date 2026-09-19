#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace crs
{
  struct UiLogEntry
  {
    std::string level;
    std::string message;
    uint32_t count = 1;
  };

  struct UiLogSyncState
  {
    uint64_t generation = 0;
    size_t rendered_entries = 0;
    uint32_t last_rendered_count = 0;
  };

  void ui_log_push(std::string_view level, std::string_view message);
  void ui_log_clear();

  // Sync collapsed log rows into the UI.
  // If out_reset is true, wipe the log list and treat out_new as the full set.
  // Otherwise append out_new, and if out_last_count > 0 update the trailing bubble.
  void ui_log_sync(UiLogSyncState &state, std::vector<UiLogEntry> &out_new, uint32_t &out_last_count, bool &out_reset);
} // namespace crs
