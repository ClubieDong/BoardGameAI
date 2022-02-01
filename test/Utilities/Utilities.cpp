#include "../../src/Utilities/Utilities.hpp"
#include <gtest/gtest.h>

TEST(Utilities, Json2Board) {
    { // Success
        const auto [board, count] = Util::Json2Board<3, 4, 2>({
            {0, 0, 0, 0},
            {1, 1, 1, 1},
            {2, 2, 2, 2},
        });
        EXPECT_EQ(count, 8);
        EXPECT_EQ(board, (decltype(board){0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2}));
    }
    { // The number of board rows does not match
        EXPECT_ANY_THROW((Util::Json2Board<3, 4, 2>({
            {0, 0, 0, 0},
            {1, 1, 1, 1},
        })));
    }
    { // The number of board columns does not match
        EXPECT_ANY_THROW((Util::Json2Board<3, 4, 2>({
            {0, 0, 0},
            {1, 1, 1, 1},
            {2, 2, 2, 2},
        })));
    }
    { // The grid value exceeds the number of players
        EXPECT_ANY_THROW((Util::Json2Board<3, 4, 2>({
            {0, 0, 0, 0},
            {1, 1, 1, 1},
            {3, 3, 3, 3},
        })));
    }
}
