#include <zk_snark/version.hpp>

#include <gtest/gtest.h>

// Wiring check only: confirms the executable links and calls the project library.
TEST(BuildSmoke, ExposesConfiguredVersion) {
    EXPECT_EQ(zk_snark::version(), ZK_SNARK_EXPECTED_VERSION);
}
