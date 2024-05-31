#include <gtest/gtest.h>
#include "Tools/DataPackage.h"

namespace tools
{
    class DataPackageTest : public ::testing::Test
    {
    protected:
        DataPackageTest() : dp(42) {}
        DataPackage dp;
    };

    TEST_F(DataPackageTest, ConstructorInitializesIdentifier)
    {
        EXPECT_EQ(dp.id(), 42);
    }

    TEST_F(DataPackageTest, IsIdReturnsTrueForMatchingId)
    {
        EXPECT_TRUE(dp.isId(42));
    }

    TEST_F(DataPackageTest, IsIdReturnsFalseForNonMatchingId)
    {
        EXPECT_FALSE(dp.isId(43));
    }

    class ComplexDataPackageTest : public ::testing::Test
    {
    protected:
        using TestPackage = ComplexDataPackage<uint32_t, int, std::string>;

        ComplexDataPackageTest() : cdp(42) {}
        TestPackage cdp;
    };

    TEST_F(ComplexDataPackageTest, ConstructorInitializesIdentifier)
    {
        EXPECT_EQ(cdp.id(), 42);
    }

    TEST_F(ComplexDataPackageTest, SetAndGetIntValue)
    {
        cdp.set<int>(1, 100);
        EXPECT_EQ(cdp.get<int>(1), 100);
    }

    TEST_F(ComplexDataPackageTest, SetAndGetStringValue)
    {
        cdp.set<std::string>(2, "test");
        EXPECT_EQ(cdp.get<std::string>(2), "test");
    }

    TEST_F(ComplexDataPackageTest, GetReturnsDefaultForMissingKey)
    {
        EXPECT_EQ(cdp.get<int>(3), 0); // Default int value is 0
        EXPECT_EQ(cdp.get<std::string>(3), ""); // Default string value is empty
    }

    TEST_F(ComplexDataPackageTest, GetThrowsForInvalidType)
    {
        cdp.set<int>(1, 100);
        EXPECT_THROW(cdp.get<std::string>(1), std::runtime_error);
    }
} // namespace tools
