#include "ui_log.h"

#include <mutex>

namespace crs
{
  namespace
  {
    constexpr size_t kMaxEntries = 500;

    std::mutex g_mu;
    std::vector<UiLogEntry> g_entries;
    uint64_t g_generation = 1;
  } // namespace

  void ui_log_push(std::string_view level, std::string_view message)
  {
    std::lock_guard lock(g_mu);

    if (!g_entries.empty() && g_entries.back().level == level && g_entries.back().message == message)
    {
      g_entries.back().count += 1;
      return;
    }

    g_entries.push_back(UiLogEntry{ std::string(level), std::string(message), 1 });
    if (g_entries.size() > kMaxEntries)
    {
      g_entries.erase(g_entries.begin(), g_entries.begin() + static_cast<std::ptrdiff_t>(g_entries.size() - kMaxEntries));
      g_generation += 1;
    }
  }

  void ui_log_clear()
  {
    std::lock_guard lock(g_mu);
    g_entries.clear();
    g_generation += 1;
  }

  void ui_log_sync(UiLogSyncState &state, std::vector<UiLogEntry> &out_new, uint32_t &out_last_count, bool &out_reset)
  {
    out_new.clear();
    out_last_count = 0;
    out_reset = false;

    std::lock_guard lock(g_mu);

    if (state.generation != g_generation)
    {
      out_reset = true;
      state.generation = g_generation;
      state.rendered_entries = 0;
      state.last_rendered_count = 0;
      out_new = g_entries;
      state.rendered_entries = g_entries.size();
      state.last_rendered_count = g_entries.empty() ? 0 : g_entries.back().count;
      return;
    }

    if (state.rendered_entries < g_entries.size())
    {
      out_new.assign(g_entries.begin() + static_cast<std::ptrdiff_t>(state.rendered_entries), g_entries.end());
      state.rendered_entries = g_entries.size();
      state.last_rendered_count = g_entries.empty() ? 0 : g_entries.back().count;
      return;
    }

    if (!g_entries.empty() && g_entries.back().count != state.last_rendered_count)
    {
      out_last_count = g_entries.back().count;
      state.last_rendered_count = out_last_count;
    }
  }
} // namespace crs
