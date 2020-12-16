inline constexpr unsigned int operator""_ms(unsigned long long x) { return x; }
inline constexpr unsigned int operator""_sec(unsigned long long x) { return x * 1000; }
inline constexpr unsigned int operator""_min(unsigned long long x) { return x * 1000 * 60; }
inline constexpr unsigned int operator""_hr(unsigned long long x) { return x * 1000 * 60 * 60; }

inline constexpr unsigned long long operator""_B(unsigned long long x) { return x; }
inline constexpr unsigned long long operator""_KB(unsigned long long x) { return x * 1024; }
inline constexpr unsigned long long operator""_MB(unsigned long long x) { return x * 1024 * 1024; }
inline constexpr unsigned long long operator""_GB(unsigned long long x) { return x * 1024 * 1024 * 1024; }
inline constexpr unsigned long long operator""_TB(unsigned long long x) { return x * 1024 * 1024 * 1024 * 1024; }
