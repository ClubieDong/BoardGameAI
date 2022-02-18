#pragma once

class NonCopyableNonMoveable {
public:
    NonCopyableNonMoveable() = default;
    NonCopyableNonMoveable(const NonCopyableNonMoveable &) = delete;
    NonCopyableNonMoveable(NonCopyableNonMoveable &&) = delete;
    NonCopyableNonMoveable &operator=(const NonCopyableNonMoveable &) = delete;
    NonCopyableNonMoveable &operator=(NonCopyableNonMoveable &&) = delete;
};
