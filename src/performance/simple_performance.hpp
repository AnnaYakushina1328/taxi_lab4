#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace taxi::performance {

class ResponseCache final {
 public:
  struct Stats {
    std::uint64_t hits{0};
    std::uint64_t misses{0};
    std::uint64_t entries{0};

    double HitRate() const {
      const auto total = hits + misses;
      if (total == 0) {
        return 0.0;
      }
      return static_cast<double>(hits) / static_cast<double>(total);
    }
  };

  std::optional<std::string> Get(const std::string& key) {
    const auto now = Clock::now();

    std::lock_guard lock(mutex_);

    const auto it = entries_.find(key);
    if (it == entries_.end()) {
      ++misses_;
      return std::nullopt;
    }

    if (it->second.expires_at <= now) {
      entries_.erase(it);
      ++misses_;
      return std::nullopt;
    }

    ++hits_;
    return it->second.value;
  }

  void Set(const std::string& key, std::string value, std::chrono::seconds ttl) {
    const auto expires_at = Clock::now() + ttl;

    std::lock_guard lock(mutex_);

    entries_[key] = Entry{
        .value = std::move(value),
        .expires_at = expires_at,
    };
  }

  void InvalidatePrefix(const std::string& prefix) {
    std::lock_guard lock(mutex_);

    for (auto it = entries_.begin(); it != entries_.end();) {
      if (it->first.rfind(prefix, 0) == 0) {
        it = entries_.erase(it);
      } else {
        ++it;
      }
    }
  }

  Stats GetStats() const {
    std::lock_guard lock(mutex_);

    return Stats{
        .hits = hits_,
        .misses = misses_,
        .entries = entries_.size(),
    };
  }

 private:
  using Clock = std::chrono::steady_clock;

  struct Entry {
    std::string value;
    Clock::time_point expires_at;
  };

  mutable std::mutex mutex_;
  std::unordered_map<std::string, Entry> entries_;
  std::uint64_t hits_{0};
  std::uint64_t misses_{0};
};

inline ResponseCache& GetCache() {
  static ResponseCache cache;
  return cache;
}

struct RateLimitResult {
  bool allowed{false};
  int limit{0};
  int remaining{0};
  std::int64_t reset_epoch_seconds{0};
};

class FixedWindowRateLimiter final {
 public:
  RateLimitResult Check(
      const std::string& key,
      int limit,
      std::chrono::seconds window
  ) {
    const auto now = std::chrono::system_clock::now();

    const auto now_seconds =
        std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()
        ).count();

    const auto window_seconds = window.count();

    const auto window_start =
        static_cast<std::int64_t>(
            (now_seconds / window_seconds) * window_seconds
        );

    const auto reset_at = window_start + window_seconds;

    std::lock_guard lock(mutex_);

    auto& bucket = buckets_[key];

    if (bucket.window_start != window_start) {
      bucket.window_start = window_start;
      bucket.count = 0;
    }

    if (bucket.count >= limit) {
      return RateLimitResult{
          .allowed = false,
          .limit = limit,
          .remaining = 0,
          .reset_epoch_seconds = reset_at,
      };
    }

    ++bucket.count;

    return RateLimitResult{
        .allowed = true,
        .limit = limit,
        .remaining = limit - bucket.count,
        .reset_epoch_seconds = reset_at,
    };
  }

 private:
  struct Bucket {
    std::int64_t window_start{0};
    int count{0};
  };

  std::mutex mutex_;
  std::unordered_map<std::string, Bucket> buckets_;
};

inline FixedWindowRateLimiter& GetRateLimiter() {
  static FixedWindowRateLimiter limiter;
  return limiter;
}

}  // namespace taxi::performance
