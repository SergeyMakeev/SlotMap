#include <gtest/gtest.h>
#include <slot_map.h>
#include <unordered_map>

TEST(SlotMapTest, BasicIterators)
{
    dod::slot_map32<int, 32, 0> slotMap;

    // allocate and remove some IDs (to waste a few allocation pages)
    for (size_t i = 0; i < static_cast<size_t>(decltype(slotMap)::kPageSize) * 2; i++)
    {
        for (size_t j = 0; j < static_cast<size_t>(decltype(slotMap)::key::kMaxVersion) + 10; j++)
        {
            auto id = slotMap.emplace(13);
            slotMap.erase(id);
        }
    }

    EXPECT_EQ(slotMap.size(), uint32_t(0));

    std::unordered_map<dod::slot_map32<int>::key, int> keyToValue;
    uint32_t numElements = 16384;
    for (int j = 0; j < int(numElements); j++)
    {
        auto id = slotMap.emplace(j);
        keyToValue[id] = j;
    }

    EXPECT_EQ(slotMap.size(), numElements);
    EXPECT_EQ(size_t(slotMap.size()), keyToValue.size());

    // iterate over all slot map values
    std::vector<uint8_t> isFound;
    isFound.resize(numElements, 0);

    for (const int& value : slotMap)
    {
        ASSERT_TRUE(value >= 0 && value < int(numElements));
        ASSERT_EQ(isFound[value], 0);
        isFound[value] = 1;
    }

    uint32_t sum = 0;
    for (uint8_t v : isFound)
    {
        sum += v;
    }
    EXPECT_EQ(sum, numElements);

    // iterate over all slot map keys & values
    isFound.clear();
    isFound.resize(numElements, 0);
    for (const auto& [key, value] : slotMap.items())
    {
        auto it = keyToValue.find(key);
        ASSERT_NE(it, keyToValue.end());
        ASSERT_EQ(it->second, value);
        ASSERT_TRUE(value >= 0 && value < int(numElements));
        ASSERT_EQ(isFound[value], 0);
        isFound[value] = 1;
    }

    sum = 0;
    for (uint8_t v : isFound)
    {
        sum += v;
    }
    EXPECT_EQ(sum, numElements);
}

struct CustomType
{
    int val = 0;
};

TEST(SlotMapTest, BasicIterators2)
{
    dod::slot_map<CustomType> slotMap;
    slotMap.emplace(CustomType{1});
    slotMap.emplace(CustomType{2});
    slotMap.emplace(CustomType{3});
    slotMap.emplace(CustomType{4});

    int numSteps = 0;
    int sum = 0;
    for (auto it = slotMap.begin(); it != slotMap.end(); it++)
    {
        sum += it->val;
        numSteps++;
    }
    EXPECT_EQ(sum, 10);
    EXPECT_EQ(numSteps, 4);
}

TEST(SlotMapTest, BasicIterators3)
{
    dod::slot_map<CustomType> slotMap;
    slotMap.emplace(CustomType{1});
    slotMap.emplace(CustomType{2});
    slotMap.emplace(CustomType{3});
    slotMap.emplace(CustomType{4});

    int numSteps = 0;
    int sum = 0;
    auto items = slotMap.items();
    for (auto it = items.begin(); it != items.end(); it++)
    {
        sum += it->second.get().val;
        numSteps++;
    }
    EXPECT_EQ(sum, 10);
    EXPECT_EQ(numSteps, 4);
}