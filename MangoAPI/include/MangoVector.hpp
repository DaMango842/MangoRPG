#pragma once

#include <cassert>
#include <algorithm>
#include <utility>
#include <concepts>
#include <memory>
#include <iterator>
#include <initializer_list>
#include <stdexcept>
#include <iostream>
#include <new> // for std::align_val_t

template<typename T>
class MangoVector {
private:
    T* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;

    static void checkAllocation(const void* ptr) {
        if (!ptr) {
            throw std::bad_alloc();
        }
    }

    static size_t calculateNewCapacity(size_t current, size_t min_required) noexcept {
        if (current == 0) return std::max<size_t>(1, min_required);
        const size_t growth_factor = (current < 1024) ? 2 : 3;
        size_t new_cap = current * growth_factor;
        if (new_cap < min_required) new_cap = min_required;
        return new_cap;
    }

    // **使用对齐分配**的辅助函数（确保满足 alignof(T)）
    static T* allocate_raw(size_t count) {
        if (count == 0) return nullptr;
        void* ptr = ::operator new(count * sizeof(T), std::align_val_t(alignof(T)));
        checkAllocation(ptr);
        return static_cast<T*>(ptr);
    }

    // reserveImpl 保留用于扩容（指数策略）
    void reserveImpl(size_t min_capacity) {
        if (min_capacity <= capacity_) return;

        const size_t new_capacity = calculateNewCapacity(capacity_, min_capacity);
        T* new_data = allocate_raw(new_capacity);

        size_t constructed = 0;
        try {
            for (; constructed < size_; ++constructed) {
                std::construct_at(new_data + constructed,
                    std::move_if_noexcept(data_[constructed]));
            }
        }
        catch (...) {
            for (size_t i = 0; i < constructed; ++i) {
                std::destroy_at(new_data + i);
            }
            ::operator delete(new_data, std::align_val_t(alignof(T)));
            throw;
        }

        // 销毁旧元素并释放内存
        for (size_t i = 0; i < size_; ++i) {
            std::destroy_at(data_ + i);
        }
        if (data_) {
            ::operator delete(data_, std::align_val_t(alignof(T)));
        }

        data_ = new_data;
        capacity_ = new_capacity;
    }

    // reserveExact — 精确分配（用于 shrink_to_fit 或用户要求的精确 reserve）
    void reserveExact(size_t exact_capacity) {
        if (exact_capacity == capacity_) return;

        T* new_data = nullptr;
        if (exact_capacity > 0) {
            new_data = allocate_raw(exact_capacity);
        }

        size_t constructed = 0;
        try {
            for (; constructed < size_ && constructed < exact_capacity; ++constructed) {
                std::construct_at(new_data + constructed,
                    std::move_if_noexcept(data_[constructed]));
            }
        }
        catch (...) {
            for (size_t i = 0; i < constructed; ++i) {
                std::destroy_at(new_data + i);
            }
            if (new_data) ::operator delete(new_data, std::align_val_t(alignof(T)));
            throw;
        }

        // 销毁旧元素并释放内存
        for (size_t i = 0; i < size_; ++i) {
            std::destroy_at(data_ + i);
        }
        if (data_) {
            ::operator delete(data_, std::align_val_t(alignof(T)));
        }

        data_ = new_data;
        capacity_ = exact_capacity;
        if (size_ > capacity_) size_ = capacity_;
    }

    void destroyAll() noexcept {
        for (size_t i = 0; i < size_; ++i) {
            std::destroy_at(data_ + i);
        }
        size_ = 0;
    }

public:
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // ================= 构造与析构 =================
    MangoVector() noexcept = default;

    explicit MangoVector(size_t count)
        requires std::default_initializable<T>
    {
        reserve(count);
        for (size_t i = 0; i < count; ++i) {
            emplace_back();
        }
    }

    MangoVector(size_t count, const T& value) {
        reserve(count);
        for (size_t i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    template<std::input_iterator It>
    MangoVector(It first, It last) {
        for (; first != last; ++first) {
            push_back(*first);
        }
    }

    MangoVector(std::initializer_list<T> init)
        : MangoVector(init.begin(), init.end()) {
    }

    // 拷贝构造
    MangoVector(const MangoVector& other) {
        reserve(other.size_);
        for (const auto& elem : other) {
            push_back(elem);
        }
    }

    // 移动构造
    MangoVector(MangoVector&& other) noexcept
        : data_(other.data_),
        size_(other.size_),
        capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // 析构函数
    ~MangoVector() noexcept {
        for (size_t i = 0; i < size_; ++i) {
            std::destroy_at(data_ + i);
        }
        size_ = 0;

        if (data_) {
            ::operator delete(data_, std::align_val_t(alignof(T)));
        }
    }

    // ================= 赋值操作 =================
    MangoVector& operator=(const MangoVector& other) {
        if (this != &other) {
            clear();
            reserve(other.size_);
            for (const auto& elem : other) {
                push_back(elem);
            }
        }
        return *this;
    }

    MangoVector& operator=(MangoVector&& other) noexcept {
        if (this != &other) {
            // 清理当前资源
            for (size_t i = 0; i < size_; ++i) {
                std::destroy_at(data_ + i);
            }
            if (data_) {
                ::operator delete(data_, std::align_val_t(alignof(T)));
            }

            // 接管 other 的资源
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;

            // 让 other 处于有效状态
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    MangoVector& operator=(std::initializer_list<T> init) {
        MangoVector temp(init);
        swap(temp);
        return *this;
    }

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

    // 添加一个精确 reserve API（若你需要“正好”分配）
    void reserve_exact(size_t exact_capacity) {
        if (exact_capacity != capacity_) {
            reserveExact(exact_capacity);
        }
    }

    void resize(size_t newSize) requires std::default_initializable<T> {
        if (newSize < size_) {
            for (size_t i = newSize; i < size_; ++i) {
                std::destroy_at(data_ + i);
            }
            size_ = newSize;
        }
        else if (newSize > size_) {
            reserve(newSize);
            for (size_t i = size_; i < newSize; ++i) {
                std::construct_at(data_ + i);
            }
            size_ = newSize;
        }
    }

    void resize(size_t newSize, const T& defaultValue) {
        if (newSize < size_) {
            for (size_t i = newSize; i < size_; ++i) {
                std::destroy_at(data_ + i);
            }
            size_ = newSize;
        }
        else if (newSize > size_) {
            reserve(newSize);
            for (size_t i = size_; i < newSize; ++i) {
                std::construct_at(data_ + i, defaultValue);
            }
            size_ = newSize;
        }
    }

    // 修正：shrink_to_fit 实现为真正的收缩（使用精确 reserve）
    void shrink_to_fit() {
        if (size_ < capacity_) {
            reserveExact(size_);
        }
    }

    // ================= 元素访问 =================
    T& at(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("MangoVector: Index out of range");
        }
        return data_[index];
    }

    const T& at(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("MangoVector: Index out of range");
        }
        return data_[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < size_ && "Index out of range");
        return data_[index];
    }

    const T& operator[](size_t index) const noexcept {
        assert(index < size_ && "Index out of range");
        return data_[index];
    }

    T& front() noexcept {
        assert(!empty() && "Accessing front of empty vector");
        return data_[0];
    }

    const T& front() const noexcept {
        assert(!empty() && "Accessing front of empty vector");
        return data_[0];
    }

    T& back() noexcept {
        assert(!empty() && "Accessing back of empty vector");
        return data_[size_ - 1];
    }

    const T& back() const noexcept {
        assert(!empty() && "Accessing back of empty vector");
        return data_[size_ - 1];
    }

    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

    // ================= 修改操作 =================
    void push_back(const T& value) {
        if (size_ >= capacity_) {
            reserveImpl(capacity_ + 1);
        }
        std::construct_at(data_ + size_, value);
        ++size_;
    }

    void push_back(T&& value) {
        if (size_ >= capacity_) {
            reserveImpl(capacity_ + 1);
        }
        std::construct_at(data_ + size_, std::move(value));
        ++size_;
    }

    template<typename... Args>
    T& emplace_back(Args&&... args)
        requires std::constructible_from<T, Args...>
    {
        if (size_ >= capacity_) {
            reserveImpl(capacity_ + 1);
        }
        std::construct_at(data_ + size_, std::forward<Args>(args)...);
        return data_[size_++];
    }

    void pop_back() noexcept {
        assert(!empty() && "Popping from empty vector");
        std::destroy_at(data_ + --size_);
    }

    void clear() noexcept {
        destroyAll();
    }

    // ================= 删除操作 =================
    iterator erase(const_iterator pos) {
        if (pos < begin() || pos >= end()) {
            throw std::out_of_range("MangoVector::erase: iterator out of range");
        }

        const size_t index = pos - begin();

        // 不要提前 destroy_at(data_ + index) —— 这样会导致后续 move 操作从已销毁对象读取（UB）。
        // 正确做法：把后续元素左移覆盖，然后销毁最后一个元素。
        for (size_t i = index; i + 1 < size_; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }

        // 销毁最后一个重复元素并调整 size
        std::destroy_at(data_ + size_ - 1);
        --size_;

        return begin() + index;
    }

    iterator erase(const_iterator first, const_iterator last) {
        if (first < begin() || last > end() || first > last) {
            throw std::out_of_range("MangoVector::erase: invalid iterator range");
        }

        if (first == last) return begin() + (first - begin());

        const size_t start_index = first - begin();
        const size_t end_index = last - begin();
        const size_t range_size = end_index - start_index;

        // 将 end_index..size_-1 的元素左移到 start_index..
        if (end_index < size_) {
            for (size_t i = end_index; i < size_; ++i) {
                data_[i - range_size] = std::move(data_[i]);
            }
        }

        // 销毁末尾的多余元素
        const size_t new_size = size_ - range_size;
        for (size_t i = new_size; i < size_; ++i) {
            std::destroy_at(data_ + i);
        }

        size_ = new_size;
        return begin() + start_index;
    }

    // ================= 插入操作 =================
    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }

    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }

    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        const size_t index = pos - begin();
        if (index > size_) {
            throw std::out_of_range("MangoVector::emplace: insertion position out of range");
        }

        // 确保有足够容量
        if (size_ >= capacity_) {
            reserveImpl(size_ + 1);
        }

        // 如果插入到末尾，直接构造
        if (index == size_) {
            std::construct_at(data_ + size_, std::forward<Args>(args)...);
            ++size_;
            return begin() + index;
        }

        // 向后移动元素：先在末尾构造最后一个元素（移动构造），
        // 然后从后向前使用 move-assignment 移动已有元素，最后在 index 覆盖构造新元素。
        std::construct_at(data_ + size_, std::move(data_[size_ - 1]));

        for (size_t i = size_ - 1; i > index; --i) {
            data_[i] = std::move(data_[i - 1]);
        }

        // 现在 data_[index] 仍然是已构造对象（旧值），我们需要先销毁再 placement-new 新值
        std::destroy_at(data_ + index);
        std::construct_at(data_ + index, std::forward<Args>(args)...);

        ++size_;
        return begin() + index;
    }

    // ================= 查找操作 =================
    template<typename U = T>
        requires std::equality_comparable<U>
    iterator find(const T& value) {
        for (size_t i = 0; i < size_; ++i) {
            if (data_[i] == value) {
                return begin() + i;
            }
        }
        return end();
    }

    template<typename U = T>
        requires std::equality_comparable<U>
    const_iterator find(const T& value) const {
        for (size_t i = 0; i < size_; ++i) {
            if (data_[i] == value) {
                return begin() + i;
            }
        }
        return end();
    }

    template<typename Predicate>
    iterator find_if(Predicate pred) {
        for (size_t i = 0; i < size_; ++i) {
            if (pred(data_[i])) {
                return begin() + i;
            }
        }
        return end();
    }

    template<typename Predicate>
    const_iterator find_if(Predicate pred) const {
        for (size_t i = 0; i < size_; ++i) {
            if (pred(data_[i])) {
                return begin() + i;
            }
        }
        return end();
    }

    template<typename U = T>
        requires std::equality_comparable<U>
    bool contains(const T& value) const {
        return find(value) != end();
    }

    template<typename U = T>
        requires std::equality_comparable<U>
    size_t index_of(const T& value) const {
        for (size_t i = 0; i < size_; ++i) {
            if (data_[i] == value) {
                return i;
            }
        }
        return size_;
    }

    // ================= 算法操作 =================
    template<typename Predicate>
    size_t remove_if(Predicate pred) {
        size_t new_size = 0;

        // 把不满足条件的元素左移。使用 move-assignment 覆盖已经构造的目标位置（安全）。
        for (size_t i = 0; i < size_; ++i) {
            if (!pred(data_[i])) {
                if (i != new_size) {
                    data_[new_size] = std::move(data_[i]);
                }
                ++new_size;
            }
        }

        // 销毁被移除的元素（位于 new_size .. size_-1）
        const size_t removed_count = size_ - new_size;
        for (size_t i = new_size; i < size_; ++i) {
            std::destroy_at(data_ + i);
        }

        size_ = new_size;
        return removed_count;
    }

    template<typename U = T>
        requires std::totally_ordered<U>
    void sort() {
        std::sort(begin(), end());
    }

    template<typename Compare>
    void sort(Compare comp) {
        std::sort(begin(), end(), comp);
    }

    // ================= 迭代器支持 =================
    iterator begin() noexcept { return data_; }
    iterator end() noexcept { return data_ + size_; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator end() const noexcept { return data_ + size_; }
    const_iterator cbegin() const noexcept { return data_; }
    const_iterator cend() const noexcept { return data_; }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

    // ================= 容量信息 =================
    size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    size_t capacity() const noexcept { return capacity_; }

    // ================= 调试方法 =================
    void debugState(const char* label) const {
        std::cout << "=== MangoVector Debug: " << label << " ===\n";
        std::cout << "大小: " << size_ << ", 容量: " << capacity_ << "\n";
        std::cout << "数据指针: " << static_cast<void*>(data_) << "\n";

        if (data_) {
            for (size_t i = 0; i < size_; ++i) {
                std::cout << "元素[" << i << "]: " << &data_[i];
                if constexpr (std::is_pointer_v<T>) {
                    std::cout << " -> " << data_[i];
                }
                std::cout << "\n";
            }
        }
        else {
            std::cout << "数据指针为 nullptr\n";
        }
        std::cout << "========================\n";
    }
};

// 全局swap函数
template<typename T>
void swap(MangoVector<T>& a, MangoVector<T>& b) noexcept {
    a.swap(b);
}

// 关系运算符
template<typename T>
bool operator==(const MangoVector<T>& lhs, const MangoVector<T>& rhs) {
    return lhs.size() == rhs.size() &&
        std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<typename T>
bool operator!=(const MangoVector<T>& lhs, const MangoVector<T>& rhs) {
    return !(lhs == rhs);
}

template<typename T>
bool operator<(const MangoVector<T>& lhs, const MangoVector<T>& rhs)
    requires std::totally_ordered<T>
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
        rhs.begin(), rhs.end());
}

template<typename T>
bool operator>(const MangoVector<T>& lhs, const MangoVector<T>& rhs)
    requires std::totally_ordered<T>
{
    return rhs < lhs;
}

template<typename T>
bool operator<=(const MangoVector<T>& lhs, const MangoVector<T>& rhs)
    requires std::totally_ordered<T>
{
    return !(rhs < lhs);
}

template<typename T>
bool operator>=(const MangoVector<T>& lhs, const MangoVector<T>& rhs)
    requires std::totally_ordered<T>
{
    return !(lhs < rhs);
}
