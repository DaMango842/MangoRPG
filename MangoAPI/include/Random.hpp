#pragma once

#include <random>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <MangoVector.hpp>

/**
 * @class Random
 * @brief 随机数工具类
 * @note 对于 MangoVector 的扩展功能需要定义 ENABLE_RANDOM_MANGO_VECTOR 宏
 * @par 启用扩展：
 * @code #define ENABLE_RANDOM_MANGO_VECTOR @endcode
 * 必须在包含本头文件前定义
 */
class Random {
public:
    /**
     * @brief 初始化随机数种子
     * @param s 种子值
     */
    static void seed(unsigned int s) {
        engine() = std::mt19937(s);
    }

    /**
     * @brief 生成 [0.0, 1.0) 范围内的随机浮点数
     * @return 均匀分布的随机 float
     */
    static float random() {
        return std::uniform_real_distribution<float>(0.0f, 1.0f)(engine());
    }

    /**
     * @brief 生成 [min, max] 范围内的随机整数
     * @param min 最小值（包含）
     * @param max 最大值（包含）
     * @return 均匀分布的随机 int
     */
    static int randint(int min, int max) {
        return std::uniform_int_distribution<int>(min, max)(engine());
    }

    /**
     * @brief 生成 [min, max) 范围内的随机浮点数
     * @param min 最小值（包含）
     * @param max 最大值（不包含）
     * @return 均匀分布的随机 float
     */
    static float uniform(float min, float max) {
        return std::uniform_real_distribution<float>(min, max)(engine());
    }

    /**
     * @brief 模拟硬币抛掷
     * @return 等概率返回 true 或 false
     */
    static bool coin() {
        return randint(0, 1) == 1;
    }

    /**
     * @brief 按概率返回成功事件
     * @param probability 成功概率 [0.0, 1.0]
     * @return 当 random() < probability 时返回 true
     */
    static bool chance(float probability) {
        return random() < probability;
    }

    /**
     * @brief 随机返回 +1 或 -1
     * @return 随机符号值
     */
    static int sign() {
        return coin() ? 1 : -1;
    }

    /**
     * @brief 从 vector 中随机选择一个元素
     * @tparam T 元素类型
     * @param vec 输入向量
     * @return 随机选择的元素的常量引用
     * @throws 若 vector 为空则触发 assert
     */
    template <typename T>
    static const T& choice(const std::vector<T>& vec) {
        assert(!vec.empty());
        return vec[randint(0, static_cast<int>(vec.size()) - 1)];
    }

#ifdef ENABLE_RANDOM_MANGO_VECTOR
    /**
     * @brief 从 MangoVector 中随机选择一个元素（需定义 ENABLE_RANDOM_MANGO_VECTOR）
     * @tparam T 元素类型
     * @param vec 输入向量
     * @return 随机选择的元素的常量引用
     * @throws 若 vector 为空则触发 assert
     */
    template <typename T>
    static const T& choice(const MangoVector<T>& vec) {
        assert(!vec.empty());
        return vec[randint(0, static_cast<int>(vec.size()) - 1)];
    }
#endif

    /**
     * @brief 从字符串中随机选择一个字符
     * @param str 输入字符串
     * @return 随机选择的字符
     * @throws 若字符串为空或超长则触发 assert
     */
    static char choice(const std::string& str) {
        assert(!str.empty());
        assert(str.size() <= static_cast<size_t>(INT_MAX));
        return str[randint(0, static_cast<int>(str.size()) - 1)];
    }

    /**
     * @brief 生成随机字符串
     * @param length 字符串长度
     * @param charset 可选字符集（默认包含大小写字母和数字）
     * @return 生成的随机字符串
     */
    static std::string randstr(size_t length,
        const std::string& charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") {
        std::string result;
        result.reserve(length);
        for (size_t i = 0; i < length; ++i)
            result += choice(charset);
        return result;
    }

    /**
     * @brief 随机打乱 vector 元素顺序
     * @tparam T 元素类型
     * @param vec 待打乱的向量
     */
    template <typename T>
    static void shuffle(std::vector<T>& vec) {
        std::shuffle(vec.begin(), vec.end(), engine());
    }

#ifdef ENABLE_RANDOM_MANGO_VECTOR
    /**
     * @brief 随机打乱 MangoVector 的元素顺序
     * @tparam T 容器元素类型
     * @param vec 要打乱顺序的 MangoVector 容器
     * @note 需要预先定义 ENABLE_RANDOM_MANGO_VECTOR 宏才能使用此功能
     * @par 示例：
     * @code
     * #define ENABLE_RANDOM_MANGO_VECTOR
     * MangoVector<int> vec = {1,2,3,4,5};
     * Random::shuffle(vec);  // 可能得到 {3,1,5,2,4}
     * @endcode
     * @par 复杂度：
     * 线性时间复杂度 O(n)，n 为容器大小
     * @par 线程安全：
     * 使用线程局部的随机数引擎，多线程调用安全
     * @see std::shuffle
     */
    template <typename T>
    static void shuffle(MangoVector<T>& vec) {
        std::shuffle(vec.begin(), vec.end(), engine());
    }
#endif

    /**
     * @brief 生成 [start, stop) 范围内按步长 step 的随机数
     * @param start 起始值（包含）
     * @param stop 结束值（不包含）
     * @param step 步长（非零）
     * @return 随机数
     * @throws 若 step 为 0 则触发 assert
     */
    static int randrange(int start, int stop, int step = 1) {
        assert(step != 0);
        int range = (stop - start + (step > 0 ? step - 1 : step + 1)) / step;
        return start + step * randint(0, range - 1);
    }

    /**
     * @brief 生成正态分布随机数
     * @param mean 均值（默认 0.0）
     * @param stddev 标准差（默认 1.0）
     * @return 正态分布的随机 float
     */
    static float gauss(float mean = 0.0f, float stddev = 1.0f) {
        return std::normal_distribution<float>(mean, stddev)(engine());
    }

    /**
     * @brief 从 vector 中无重复采样 n 个元素
     * @tparam T 元素类型
     * @param vec 输入向量
     * @param n 采样数量
     * @return 包含采样结果的 vector
     * @throws 若 n > vec.size() 则触发 assert
     */
    template <typename T>
    static std::vector<T> sample(const std::vector<T>& vec, size_t n) {
        assert(n <= vec.size());
        std::vector<T> copy = vec;
        shuffle(copy);
        return std::vector<T>(copy.begin(), copy.begin() + n);
    }

#ifdef ENABLE_RANDOM_MANGO_VECTOR
    /**
     * @brief 从 MangoVector 中无重复采样 n 个元素（需定义 ENABLE_RANDOM_MANGO_VECTOR）
     * @tparam T 元素类型
     * @param vec 输入向量
     * @param n 采样数量
     * @return 包含采样结果的 MangoVector
     * @throws 若 n > vec.size() 则触发 assert
     */
    template <typename T>
    static MangoVector<T> sample(const MangoVector<T>& vec, size_t n) {
        assert(n <= vec.size());
        MangoVector<T> copy = vec;
        shuffle(copy);
        return MangoVector<T>(copy.begin(), copy.begin() + n);
    }
#endif

    /**
     * @brief 带权重随机选择
     * @tparam T 元素类型
     * @param items 候选元素集合
     * @param weights 对应权重集合
     * @return 被选中的元素的常量引用
     * @throws 若 items 和 weights 大小不等则触发 assert
     */
    template <typename T>
    static const T& weighted_choice(const std::vector<T>& items, const std::vector<float>& weights) {
        assert(items.size() == weights.size());
        std::discrete_distribution<int> dist(weights.begin(), weights.end());
        return items[dist(engine())];
    }

#ifdef ENABLE_RANDOM_MANGO_VECTOR
    /**
     * @brief MangoVector 版本的带权重随机选择（需定义 ENABLE_RANDOM_MANGO_VECTOR）
     * @tparam T 元素类型
     * @param items 候选元素集合
     * @param weights 对应权重集合
     * @return 被选中的元素的常量引用
     * @throws 若 items 和 weights 大小不等则触发 assert
     */
    template <typename T>
    static const T& weighted_choice(const MangoVector<T>& items, const MangoVector<float>& weights) {
        assert(items.size() == weights.size());
        std::discrete_distribution<int> dist(weights.begin(), weights.end());
        return items[dist(engine())];
    }
#endif

private:
    /**
     * @brief 获取线程局部的随机数引擎
     * @return 静态的随机数引擎引用
     * @note 首次调用时使用 std::random_device 初始化
     */
    static std::mt19937& engine() {
        static std::random_device rd;
        static std::mt19937 rng(rd());
        return rng;
    }
};
