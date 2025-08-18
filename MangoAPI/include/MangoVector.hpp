#pragma once

#include <cassert>
#include <algorithm>
#include <utility>
#include <concepts>
#include <memory> // for std::move_if_noexcept

/**
 * @def ENABLE_RANDOM_MANGO_VECTOR
 * @brief 启用与 Random 类的集成功能
 */
#define ENABLE_RANDOM_MANGO_VECTOR

 /**
  * @class MangoVector
  * @brief 增强型 vector 容器，支持安全校验和现代 C++ 特性
  * @tparam T 元素类型
  *
  * @note 支持 C++17/20 特性：
  * - if constexpr 编译时分支
  * - 概念约束 (C++20)
  * - 移动语义优化
  */
template<typename T>
class MangoVector {
private:
    T* data_ = nullptr;      // 数据存储指针
    size_t size_ = 0;        // 当前元素数量
    size_t capacity_ = 0;    // 当前分配容量

    // 扩容策略：至少分配 min_capacity，否则按几何增长（2倍）
    void reserveImpl(size_t min_capacity) {
        if (min_capacity <= capacity_) return;

        // 计算新容量（至少为当前容量的2倍）
        size_t new_capacity = capacity_ ? capacity_ * 2 : 1;
        if (new_capacity < min_capacity) new_capacity = min_capacity;

        // 分配新内存
        T* new_data = static_cast<T*>(::operator new(new_capacity * sizeof(T)));
        size_t new_size = 0;

        try {
            // 转移元素到新内存（优先移动，不可移动则拷贝）
            for (size_t i = 0; i < size_; ++i) {
                new (new_data + i) T(std::move_if_noexcept(data_[i]));
                ++new_size;
            }
        }
        catch (...) {
            // 异常回滚：析构已构造元素并释放内存
            for (size_t j = 0; j < new_size; ++j) {
                new_data[j].~T();
            }
            ::operator delete(new_data);
            throw;
        }

        // 销毁旧元素并释放旧内存
        destroyAll();
        ::operator delete(data_);

        // 更新指针和容量
        data_ = new_data;
        capacity_ = new_capacity;
    }

    // 销毁所有元素（不释放内存）
    void destroyAll() {
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        size_ = 0;
    }

    // 元素有效性检查
    void assertValid(const T& value) const {
        if constexpr (requires { value.isValid(); }) {
            assert(value.isValid() && "MangoVector: Attempted to insert invalid object!");
        }
    }

public:
    // ================= 构造与析构 =================
    MangoVector() = default;

    MangoVector(const MangoVector& other) : size_(other.size_), capacity_(other.size_) {
        if (capacity_ > 0) {
            data_ = static_cast<T*>(::operator new(capacity_ * sizeof(T)));
            for (size_t i = 0; i < size_; ++i) {
                new (data_ + i) T(other.data_[i]); // 拷贝构造
            }
        }
    }

    MangoVector(MangoVector&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    ~MangoVector() {
        destroyAll();
        ::operator delete(data_);
    }

    // ================= 赋值操作 =================
    MangoVector& operator=(const MangoVector& other) {
        if (this != &other) {
            // 拷贝并交换 (copy-and-swap)
            MangoVector temp(other);
            swap(temp);
        }
        return *this;
    }

    MangoVector& operator=(MangoVector&& other) noexcept {
        if (this != &other) {
            // 直接接管资源
            destroyAll();
            ::operator delete(data_);

            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;

            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    // 交换两个容器
    void swap(MangoVector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // ================= 容量操作 =================
    void reserve(size_t capacity) {
        if (capacity > capacity_) {
            reserveImpl(capacity);
        }
    }

    void resize(size_t newSize) requires std::default_initializable<T> {
        if (newSize < size_) {
            // 销毁多余元素
            for (size_t i = newSize; i < size_; ++i) {
                data_[i].~T();
            }
            size_ = newSize;
        }
        else if (newSize > size_) {
            reserve(newSize);
            // 默认构造新元素
            for (size_t i = size_; i < newSize; ++i) {
                new (data_ + i) T();
            }
            size_ = newSize;
        }
    }

    void resize(size_t newSize, const T& defaultValue) {
        if (newSize < size_) {
            // 销毁多余元素
            for (size_t i = newSize; i < size_; ++i) {
                data_[i].~T();
            }
            size_ = newSize;
        }
        else if (newSize > size_) {
            reserve(newSize);
            // 用默认值构造新元素
            for (size_t i = size_; i < newSize; ++i) {
                new (data_ + i) T(defaultValue);
            }
            size_ = newSize;
        }
    }

    // ================= 元素访问 =================
    T& at(size_t index) {
        assert(index < size_ && "Index out of range");
        return data_[index];
    }

    const T& at(size_t index) const {
        assert(index < size_ && "Index out of range");
        return data_[index];
    }

    T& operator[](size_t index) noexcept {
        return data_[index];
    }

    const T& operator[](size_t index) const noexcept {
        return data_[index];
    }

    // ================= 修改操作 =================
    void push_back(const T& value) {
        assertValid(value);
        if (size_ == capacity_) {
            reserveImpl(capacity_ ? capacity_ * 2 : 1);
        }
        new (data_ + size_) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        assertValid(value);
        if (size_ == capacity_) {
            reserveImpl(capacity_ ? capacity_ * 2 : 1);
        }
        new (data_ + size_) T(std::move(value));
        ++size_;
    }

    template<typename... Args>
    T& emplace_back(Args&&... args)
        requires std::constructible_from<T, Args...>
    {
        if constexpr (requires { T(std::forward<Args>(args)...).isValid(); }) {
            // 构造临时对象进行有效性检查
            T temp(std::forward<Args>(args)...);
            assertValid(temp);

            if (size_ == capacity_) {
                reserveImpl(capacity_ ? capacity_ * 2 : 1);
            }
            new (data_ + size_) T(std::move(temp));
        }
        else {
            if (size_ == capacity_) {
                reserveImpl(capacity_ ? capacity_ * 2 : 1);
            }
            new (data_ + size_) T(std::forward<Args>(args)...);
        }
        return data_[size_++];
    }

    void pop_back() {
        assert(size_ > 0 && "Vector is empty");
        data_[--size_].~T();
    }

    void removeAt(size_t index) {
        assert(index < size_ && "Index out of range");
        // 移动覆盖要删除的元素
        for (size_t i = index; i < size_ - 1; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }
        // 销毁最后一个元素
        data_[--size_].~T();
    }

    // ================= 查找操作 =================
    bool contains(const T& value) const
        requires std::equality_comparable<T>
    {
        for (size_t i = 0; i < size_; ++i) {
            if (data_[i] == value) return true;
        }
        return false;
    }

    // ================= 迭代器支持 =================
    T* begin() noexcept { return data_; }
    T* end() noexcept { return data_ + size_; }
    const T* begin() const noexcept { return data_; }
    const T* end() const noexcept { return data_ + size_; }
    const T* cbegin() const noexcept { return data_; }
    const T* cend() const noexcept { return data_ + size_; }

    // ================= 容量信息 =================
    size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    size_t capacity() const noexcept { return capacity_; }

    void clear() noexcept {
        destroyAll();
    }

    // ================= 高级操作 =================
    template<typename Predicate>
    void removeIf(Predicate pred) {
        T* write_ptr = data_;
        for (T* read_ptr = data_; read_ptr != data_ + size_; ++read_ptr) {
            if (!pred(*read_ptr)) {
                if (write_ptr != read_ptr) {
                    *write_ptr = std::move(*read_ptr);
                }
                ++write_ptr;
            }
        }

        // 销毁被移除的元素
        size_t new_size = write_ptr - data_;
        for (size_t i = new_size; i < size_; ++i) {
            data_[i].~T();
        }
        size_ = new_size;
    }
};
