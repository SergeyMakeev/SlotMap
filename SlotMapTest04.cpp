#include <gtest/gtest.h>
#include <map>
#include <slot_map.h>
#include <unordered_map>

// Note: You could run "SlotMapTest --gtest_filter=SlotMapTest.PageDeactivationOnClear" if you need to run only a specific test
TEST(SlotMapTest, IdsToAssociativeContainers64)
{
    dod::slot_map64<int> slotMap;

    std::unordered_map<dod::slot_map64<int>::key, int> unorderedMap;
    std::map<dod::slot_map64<int>::key, int> map;

    for (int i = 0; i < 128; i++)
    {
        auto id = slotMap.emplace(i);
        unorderedMap.emplace(std::make_pair(id, i));
        map.emplace(std::make_pair(id, i));
    }
    EXPECT_EQ(slotMap.size(), 128u);
    EXPECT_EQ(unorderedMap.size(), size_t(128));
    EXPECT_EQ(map.size(), size_t(128));

    for (const auto& [key, value] : slotMap.items())
    {
        auto uit = unorderedMap.find(key);
        ASSERT_NE(uit, unorderedMap.end());
        ASSERT_EQ(uit->second, value);

        auto it = map.find(key);
        ASSERT_NE(it, map.end());
        ASSERT_EQ(it->second, value);
    }
}

TEST(SlotMapTest, IdsToAssociativeContainers32)
{
    dod::slot_map32<int> slotMap;

    std::unordered_map<dod::slot_map32<int>::key, int> unorderedMap;
    std::map<dod::slot_map32<int>::key, int> map;

    for (int i = 0; i < 128; i++)
    {
        auto id = slotMap.emplace(i);
        unorderedMap.emplace(std::make_pair(id, i));
        map.emplace(std::make_pair(id, i));
    }
    EXPECT_EQ(slotMap.size(), 128u);
    EXPECT_EQ(unorderedMap.size(), size_t(128));
    EXPECT_EQ(map.size(), size_t(128));

    for (const auto& [key, value] : slotMap.items())
    {
        auto uit = unorderedMap.find(key);
        ASSERT_NE(uit, unorderedMap.end());
        ASSERT_EQ(uit->second, value);

        auto it = map.find(key);
        ASSERT_NE(it, map.end());
        ASSERT_EQ(it->second, value);
    }
}

TEST(SlotMapTest, SlotsDeactivationOnClear)
{
    dod::slot_map32<int> slotMap;
    for (int i = 0; i < static_cast<int>(decltype(slotMap)::key::kMaxVersion) + 10; i++)
    {
        for (size_t k = 0; k < 128; k++)
        {
            slotMap.emplace(1);
        }
        EXPECT_EQ(slotMap.size(), 128u);
        slotMap.clear();
        EXPECT_TRUE(slotMap.empty());
    }

    slotMap.emplace(13);
}

TEST(SlotMapTest, PageDeactivationOnClear)
{
    dod::slot_map32<int, 32, 0> slotMap;
    size_t numIterations = static_cast<size_t>(decltype(slotMap)::key::kMaxVersion) + 10;
    for (size_t j = 0; j < numIterations; j++)
    {
        size_t numItemsToAllocate = static_cast<size_t>(decltype(slotMap)::kPageSize) * 4;
        for (size_t i = 0; i < numItemsToAllocate; i++)
        {
            slotMap.emplace(1);
        }

        EXPECT_EQ(slotMap.size(), numItemsToAllocate);
        slotMap.clear();
        EXPECT_TRUE(slotMap.empty());
    }

    slotMap.emplace(13);
}

TEST(SlotMapTest, InvalidAndMalformedKeys)
{
    dod::slot_map<int> slotMap;
    slotMap.emplace(1);
    slotMap.emplace(2);
    slotMap.emplace(3);
    dod::slot_map<int>::key invalidKey = dod::slot_map<int>::key::invalid();

    const int* val1 = slotMap.get(invalidKey);
    EXPECT_EQ(val1, nullptr);

    EXPECT_EQ(slotMap.has_key(invalidKey), false);

    std::optional<int> opt1 = slotMap.pop(invalidKey);
    EXPECT_EQ(opt1.has_value(), false);
    slotMap.erase(invalidKey);

    dod::slot_map<int>::key malformedKey;
    malformedKey.raw = 0xffffffffffffffffull;

    const int* val2 = slotMap.get(malformedKey);
    EXPECT_EQ(val2, nullptr);

    EXPECT_EQ(slotMap.has_key(malformedKey), false);

    std::optional<int> opt2 = slotMap.pop(malformedKey);
    EXPECT_EQ(opt2.has_value(), false);
    slotMap.erase(malformedKey);

    dod::slot_map<int>::key zeroKey;
    zeroKey.raw = 0x0ull;

    const int* val3 = slotMap.get(zeroKey);
    EXPECT_EQ(val3, nullptr);

    EXPECT_EQ(slotMap.has_key(zeroKey), false);

    std::optional<int> opt3 = slotMap.pop(zeroKey);
    EXPECT_EQ(opt3.has_value(), false);
    slotMap.erase(zeroKey);
}

TEST(SlotMapTest, WorkingWithRemovedPages)
{
    dod::slot_map32<int, 32, 0> slotMap;

    auto id = slotMap.emplace(33);
    slotMap.erase(id);

    // allocate and remove some IDs (to waste a few allocation pages)
    for (size_t i = 0; i < static_cast<size_t>(decltype(slotMap)::kPageSize) * 2; i++)
    {
        for (size_t j = 0; j < static_cast<size_t>(decltype(slotMap)::key::kMaxVersion) + 10; j++)
        {
            auto tmpId = slotMap.emplace(13);
            std::optional<int> val = slotMap.pop(tmpId);
            EXPECT_TRUE(val.has_value());
        }
    }

    EXPECT_EQ(slotMap.size(), 0u);

    // add something to the slot map to pass index check
    for (size_t i = 0; i < 128; i++)
    {
        slotMap.emplace(44);
    }

    const int* v = slotMap.get(id);
    EXPECT_EQ(v, nullptr);

    slotMap.erase(id);

    slotMap.pop(id);

    // allocate and remove some items but less that a whole page
    for (size_t j = 0; j < static_cast<size_t>(decltype(slotMap)::key::kMaxVersion) + 10; j++)
    {
        auto tmpId = slotMap.emplace(13);
        std::optional<int> val = slotMap.pop(tmpId);
        EXPECT_TRUE(val.has_value());
    }

    auto stats = slotMap.debug_stats();

    printf("Pages: total: %d, active: %d, inactive: %d\n", int(stats.numPagesTotal), int(stats.numActivePages),
           int(stats.numInactivePages));
    printf("Items: total: %d, alive: %d, tombstone: %d, inactive: %d\n", int(stats.numItemsTotal), int(stats.numAliveItems),
           int(stats.numTombstoneItems), int(stats.numInactiveItems));
}

TEST(SlotMapTest, RecursiveDeclaration)
{
    struct HalfEdge;
    using HalfEdgeSM = dod::slot_map32<HalfEdge>;

    struct HalfEdge
    {
        int value = 0;
        HalfEdgeSM::key left = HalfEdgeSM::key::invalid();
        HalfEdgeSM::key right = HalfEdgeSM::key::invalid();

        HalfEdge() = default;
        HalfEdge(int _value)
            : value(_value)
        {
        }
    };

    HalfEdgeSM slotMap;

    HalfEdge a(1);
    HalfEdge b(2);
    HalfEdge c(3);

    c.left = slotMap.emplace(a);
    c.right = slotMap.emplace(b);

    EXPECT_TRUE(slotMap.has_key(c.left));
    EXPECT_TRUE(slotMap.has_key(c.right));

    EXPECT_EQ(c.value, 3);
    EXPECT_EQ(slotMap.get(c.left)->value, 1);
    EXPECT_EQ(slotMap.get(c.right)->value, 2);
}

// This is a compiler test - not the real unit-test
#if 0
TEST(SlotMapTest, CompileTimeTypeSafety)
{
    using TSlotMapInt = dod::slot_map32<int>;
    using TSlotMapFloat = dod::slot_map32<float>;

    TSlotMapInt::key keyInt = TSlotMapInt::key::invalid();
    TSlotMapFloat::key keyFloat = TSlotMapFloat::key::invalid();

    keyInt = keyInt;      // OK
    keyFloat == keyFloat; // OK

    // should not compile!
    keyInt = keyFloat;

    if (keyInt == keyInt) // OK
    {
    }

    if (keyFloat == keyFloat) // OK
    {
    }

    // should not compile!
    if (keyInt == keyFloat) 
    {
    }
}
#endif