#pragma once

#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>
#include <memory>
#include <compare>
#include <format>
#include <algorithm>
#include <limits>
#include <iostream>
#include <type_traits>
#include <concepts>
#include <iterator>
#include <utility>
#include <sstream>
#include <functional>
#include <array>
#include <new>

#if __cplusplus >= 202002L
#define MANGO_STRING_CONSTEXPR constexpr
#define MANGO_STRING_CONSTEVAL consteval
#else
#define MANGO_STRING_CONSTEXPR
#define MANGO_STRING_CONSTEVAL constexpr
#endif

#if defined(MANGO_STRING_ENABLE_SFML) || __has_include(<SFML/System/String.hpp>)
#include <SFML/System/String.hpp>
#define MANGO_STRING_SFML_SUPPORT
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>>
class MangoString;

using String = MangoString<char>;
using WString = MangoString<wchar_t>;
using U8String = MangoString<char8_t>;
using U16String = MangoString<char16_t>;
using U32String = MangoString<char32_t>;

template <typename CharT, typename Traits>
class MangoString {
public:
    using value_type = CharT;
    using traits_type = Traits;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using reference = CharT&;
    using const_reference = const CharT&;
    using pointer = CharT*;
    using const_pointer = const CharT*;
    using iterator = CharT*;
    using const_iterator = const CharT*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using view_type = std::basic_string_view<CharT, Traits>;

    static constexpr size_type npos = static_cast<size_type>(-1);

private:
    // 统一的堆存储结构
    struct HeapStorage {
        CharT* ptr;
        size_t size;
        size_t capacity;

        constexpr HeapStorage() noexcept : ptr(nullptr), size(0), capacity(0) {}
    };

    // 内部存储策略
    class Storage {
    private:
        static constexpr size_t MAX_SSO_SIZE = []() {
            constexpr size_t CONTROL_SIZE = sizeof(uintptr_t);
            constexpr size_t BUFFER_SIZE = sizeof(uintptr_t) * 3;

            constexpr size_t capacity = (BUFFER_SIZE - CONTROL_SIZE) / sizeof(CharT) - 1;

            if constexpr (capacity > 63) {
                return 63;
            }
            else {
                return capacity;
            }
            }();

        union Data {
            struct {
                CharT buffer[MAX_SSO_SIZE + 1];
            } sso;

            HeapStorage heap;

            constexpr Data() noexcept : sso{} {}
        } data_;

        // 控制信息：使用指针的最后一位标记SSO状态
        uintptr_t control_;

        static constexpr uintptr_t SSO_FLAG = 0x1;
        static constexpr uintptr_t PTR_MASK = ~SSO_FLAG;

        MANGO_STRING_CONSTEXPR bool is_sso() const noexcept {
            return (control_ & SSO_FLAG) != 0;
        }

        MANGO_STRING_CONSTEXPR size_t sso_size() const noexcept {
            return control_ >> 1;
        }

        MANGO_STRING_CONSTEXPR void set_sso(bool sso, size_t size = 0) noexcept {
            if (sso) {
                control_ = (size << 1) | SSO_FLAG;
            }
            else {
                control_ = 0;
            }
        }

    public:
        // 构造函数
        MANGO_STRING_CONSTEXPR Storage() noexcept : data_(), control_((0 << 1) | SSO_FLAG) {
            data_.sso.buffer[0] = CharT();
        }

        // 禁止拷贝
        Storage(const Storage&) = delete;
        Storage& operator=(const Storage&) = delete;

        // 移动构造
        MANGO_STRING_CONSTEXPR Storage(Storage&& other) noexcept : data_(), control_(other.control_) {
            if (is_sso()) {
                const size_t size = sso_size();
                Traits::copy(data_.sso.buffer, other.data_.sso.buffer, size + 1);
            }
            else {
                data_.heap = other.data_.heap;
                other.set_sso(true, 0);
                other.data_.sso.buffer[0] = CharT();
            }
        }

        // 移动赋值
        MANGO_STRING_CONSTEXPR Storage& operator=(Storage&& other) noexcept {
            if (this != &other) {
                destroy();

                control_ = other.control_;
                if (is_sso()) {
                    const size_t size = sso_size();
                    Traits::copy(data_.sso.buffer, other.data_.sso.buffer, size + 1);
                }
                else {
                    data_.heap = other.data_.heap;
                    other.set_sso(true, 0);
                    other.data_.sso.buffer[0] = CharT();
                }
            }
            return *this;
        }

        ~Storage() {
            destroy();
        }

        // 销毁资源
        MANGO_STRING_CONSTEXPR void destroy() noexcept {
            if (!is_sso() && data_.heap.ptr) {
                delete[] data_.heap.ptr;
                data_.heap.ptr = nullptr;
                data_.heap.size = 0;
                data_.heap.capacity = 0;
            }
        }

        // 容量管理
        MANGO_STRING_CONSTEXPR size_t size() const noexcept {
            return is_sso() ? sso_size() : data_.heap.size;
        }

        MANGO_STRING_CONSTEXPR size_t capacity() const noexcept {
            return is_sso() ? MAX_SSO_SIZE : data_.heap.capacity;
        }

        MANGO_STRING_CONSTEXPR bool empty() const noexcept {
            return size() == 0;
        }

        // 数据访问
        MANGO_STRING_CONSTEXPR CharT* data() noexcept {
            return is_sso() ? data_.sso.buffer : data_.heap.ptr;
        }

        MANGO_STRING_CONSTEXPR const CharT* data() const noexcept {
            return is_sso() ? data_.sso.buffer : data_.heap.ptr;
        }

        // 分配存储
        template<typename Func>
        void allocate(size_t count, Func&& initializer) {
            if (count > max_size()) {
                throw std::length_error("MangoString: allocation exceeds max size");
            }

            if (count <= MAX_SSO_SIZE) {
                if (!is_sso()) {
                    destroy();
                }
                initializer(data_.sso.buffer, count);
                data_.sso.buffer[count] = CharT();
                set_sso(true, count);
            }
            else {
                HeapStorage new_heap;
                new_heap.ptr = new (std::nothrow) CharT[count + 1];
                if (!new_heap.ptr) {
                    throw std::bad_alloc();
                }

                try {
                    initializer(new_heap.ptr, count);
                    new_heap.ptr[count] = CharT();
                    new_heap.size = count;
                    new_heap.capacity = count;

                    destroy();
                    data_.heap = new_heap;
                    set_sso(false);
                }
                catch (...) {
                    delete[] new_heap.ptr;
                    throw;
                }
            }
        }

        // 预留容量
        void reserve(size_t new_capacity) {
            if (new_capacity <= capacity()) return;

            const size_t current_size = size();
            CharT* new_data = new (std::nothrow) CharT[new_capacity + 1];
            if (!new_data) {
                throw std::bad_alloc();
            }

            try {
                Traits::copy(new_data, data(), current_size + 1);
                destroy();

                data_.heap.ptr = new_data;
                data_.heap.size = current_size;
                data_.heap.capacity = new_capacity;
                set_sso(false);
            }
            catch (...) {
                delete[] new_data;
                throw;
            }
        }

        // 设置大小
        MANGO_STRING_CONSTEXPR void set_size(size_t new_size) noexcept {
            if (is_sso()) {
                set_sso(true, new_size);
                data_.sso.buffer[new_size] = CharT();
            }
            else {
                data_.heap.size = new_size;
                data_.heap.ptr[new_size] = CharT();
            }
        }

        // 最大大小
        static constexpr size_t max_size() noexcept {
            return std::numeric_limits<size_t>::max() / 2;
        }

        // 交换
        MANGO_STRING_CONSTEXPR void swap(Storage& other) noexcept {
            using std::swap;

            if (is_sso() && other.is_sso()) {
                const size_t max_size = std::max(sso_size(), other.sso_size());
                for (size_t i = 0; i <= max_size; ++i) {
                    swap(data_.sso.buffer[i], other.data_.sso.buffer[i]);
                }
                swap(control_, other.control_);
            }
            else if (!is_sso() && !other.is_sso()) {
                swap(data_.heap, other.data_.heap);
                swap(control_, other.control_);
            }
            else {
                Storage temp(std::move(*this));
                *this = std::move(other);
                other = std::move(temp);
            }
        }

        // 获取SSO最大容量
        static constexpr size_t sso_capacity() noexcept {
            return MAX_SSO_SIZE;
        }
    };

    Storage storage_;

private:
    // 具体的构造实现
    MANGO_STRING_CONSTEXPR void construct_from_cstring(const CharT* str, size_type count) {
        if (!str) return;
        const size_type len = (count == npos) ? Traits::length(str) : count;
        if (len > 0) {
            storage_.allocate(len, [&](CharT* dest, size_type c) {
                Traits::copy(dest, str, c);
                });
        }
    }

    MANGO_STRING_CONSTEXPR void construct_from_mango_string(const MangoString& other) {
        if (!other.empty()) {
            storage_.allocate(other.size(), [&](CharT* dest, size_type count) {
                Traits::copy(dest, other.data(), count);
                });
        }
    }

    MANGO_STRING_CONSTEXPR void construct_from_view(view_type sv) {
        if (!sv.empty()) {
            storage_.allocate(sv.size(), [&](CharT* dest, size_type count) {
                Traits::copy(dest, sv.data(), count);
                });
        }
    }

    MANGO_STRING_CONSTEXPR void construct_from_fill(size_type count, CharT ch) {
        if (count > 0) {
            storage_.allocate(count, [&](CharT* dest, size_type c) {
                Traits::assign(dest, c, ch);
                });
        }
    }

    MANGO_STRING_CONSTEXPR void construct_from_initializer_list(std::initializer_list<CharT> ilist) {
        if (!ilist.empty()) {
            storage_.allocate(ilist.size(), [&](CharT* dest, size_type count) {
                Traits::copy(dest, ilist.begin(), count);
                });
        }
    }

    template<typename InputIt>
    void construct_from_range(InputIt first, InputIt last) {
        // 对于连续迭代器优化
        if constexpr (std::contiguous_iterator<InputIt>) {
            if (first != last) {
                const size_type count = std::distance(first, last);
                storage_.allocate(count, [&](CharT* dest, size_type c) {
                    Traits::copy(dest, std::to_address(first), c);
                    });
            }
        }
        else {
            // 通用迭代器处理
            for (; first != last; ++first) {
                push_back(*first);
            }
        }
    }

    // 统一的赋值实现
    MANGO_STRING_CONSTEXPR MangoString& assign_impl(const CharT* data, size_type count) {
        if (count == 0) {
            clear();
        }
        else {
            storage_.allocate(count, [&](CharT* dest, size_type c) {
                Traits::copy(dest, data, c);
                });
        }
        return *this;
    }

    // 统一的追加实现
    MANGO_STRING_CONSTEXPR MangoString& append_impl(const CharT* data, size_type count) {
        if (count > 0) {
            const size_type current_size = size();
            const size_type new_size = current_size + count;

            if (new_size > capacity()) {
                reserve(std::max(new_size, capacity() * 2));
            }

            Traits::copy(this->data() + current_size, data, count);
            this->data()[new_size] = CharT();
            storage_.set_size(new_size);
        }
        return *this;
    }

    MANGO_STRING_CONSTEXPR MangoString& append_fill_impl(size_type count, CharT ch) {
        if (count > 0) {
            const size_type current_size = size();
            const size_type new_size = current_size + count;

            if (new_size > capacity()) {
                reserve(std::max(new_size, capacity() * 2));
            }

            Traits::assign(this->data() + current_size, count, ch);
            this->data()[new_size] = CharT();
            storage_.set_size(new_size);
        }
        return *this;
    }

    // 统一的查找实现
    MANGO_STRING_CONSTEXPR size_type find_char_impl(CharT ch, size_type pos) const noexcept {
        for (size_type i = pos; i < size(); ++i) {
            if (Traits::eq(data()[i], ch)) {
                return i;
            }
        }
        return npos;
    }

    MANGO_STRING_CONSTEXPR size_type find_string_impl(const CharT* str, size_type pos, size_type count) const noexcept {
        if (!str || count == 0 || pos > size()) return npos;

        const size_type len = size();
        if (pos + count > len) return npos;

        for (size_type i = pos; i <= len - count; ++i) {
            if (Traits::compare(data() + i, str, count) == 0) {
                return i;
            }
        }
        return npos;
    }

public:
    // 统一的构造函数
    MANGO_STRING_CONSTEXPR MangoString() noexcept = default;

    // 拷贝构造函数
    MANGO_STRING_CONSTEXPR MangoString(const MangoString& other) {
        construct_from_mango_string(other);
    }

    // 移动构造函数
    MANGO_STRING_CONSTEXPR MangoString(MangoString&& other) noexcept
        : storage_(std::move(other.storage_)) {
    }

    // 主构造函数 - 统一入口
    template<typename T>
        requires std::constructible_from<view_type, T> ||
    std::is_same_v<std::remove_cvref_t<T>, std::initializer_list<CharT>>
        MANGO_STRING_CONSTEXPR MangoString(T&& value) {
        using RawT = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<RawT, std::initializer_list<CharT>>) {
            construct_from_initializer_list(value);
        }
        else if constexpr (requires { view_type(value); }) {
            construct_from_view(view_type(value));
        }
    }

    // 带长度的构造函数
    template<typename T>
        requires std::convertible_to<T, const CharT*>
    MANGO_STRING_CONSTEXPR MangoString(T&& data, size_type count)
        : MangoString() {
        construct_from_cstring(static_cast<const CharT*>(data), count);
    }

    // 填充构造函数
    MANGO_STRING_CONSTEXPR MangoString(size_type count, CharT ch)
        : MangoString() {
        construct_from_fill(count, ch);
    }

    // 迭代器范围构造函数
    template<typename InputIt>
        requires (!std::constructible_from<view_type, InputIt>) &&
    std::input_iterator<InputIt>
        MangoString(InputIt first, InputIt last) : MangoString() {
        construct_from_range(first, last);
    }

#ifdef MANGO_STRING_SFML_SUPPORT
    MangoString(const sf::String& sfStr) : MangoString() {
        if constexpr (std::is_same_v<CharT, char>) {
            std::string temp = sfStr.toAnsiString();
            assign(temp.data(), temp.size());
        }
        else if constexpr (std::is_same_v<CharT, wchar_t>) {
            std::wstring temp = sfStr.toWideString();
            assign(temp.data(), temp.size());
        }
        else {
            std::u32string temp = sfStr.toUtf32();
            assign(temp.data(), temp.size());
        }
    }
#endif

    // 析构函数
    ~MangoString() = default;

    // 拷贝赋值操作符
    MANGO_STRING_CONSTEXPR MangoString& operator=(const MangoString& other) {
        if (this != &other) {
            if (other.empty()) {
                clear();
            }
            else {
                storage_.allocate(other.size(), [&](CharT* dest, size_type count) {
                    Traits::copy(dest, other.data(), count);
                    });
            }
        }
        return *this;
    }

    // 移动赋值操作符
    MANGO_STRING_CONSTEXPR MangoString& operator=(MangoString&& other) noexcept {
        if (this != &other) {
            storage_ = std::move(other.storage_);
        }
        return *this;
    }

    // 统一赋值操作符
    template<typename T>
        requires std::constructible_from<view_type, T> &&
    (!std::is_same_v<std::remove_cvref_t<T>, MangoString>) &&
        std::is_same_v<std::remove_cvref_t<T>, CharT>
        MANGO_STRING_CONSTEXPR MangoString& operator=(T&& value) {
        using RawT = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<RawT, CharT>) {
            assign(1, value);
        }
        else {
            assign(view_type(value));
        }
        return *this;
    }

    // 容量相关
    MANGO_STRING_CONSTEXPR size_type size() const noexcept { return storage_.size(); }
    MANGO_STRING_CONSTEXPR size_type length() const noexcept { return size(); }
    MANGO_STRING_CONSTEXPR bool empty() const noexcept { return storage_.empty(); }
    MANGO_STRING_CONSTEXPR size_type capacity() const noexcept { return storage_.capacity(); }
    MANGO_STRING_CONSTEXPR size_type max_size() const noexcept { return storage_.max_size(); }

    void reserve(size_type new_capacity) {
        storage_.reserve(new_capacity);
    }

    void shrink_to_fit() {
        const size_type current_size = size();
        if (current_size < capacity()) {
            if (current_size == 0) {
                storage_ = Storage();
            }
            else {
                storage_.allocate(current_size, [&](CharT* dest, size_type count) {
                    Traits::copy(dest, data(), count);
                    });
            }
        }
    }

    // 元素访问
    MANGO_STRING_CONSTEXPR reference operator[](size_type pos) noexcept {
        return data()[pos];
    }

    MANGO_STRING_CONSTEXPR const_reference operator[](size_type pos) const noexcept {
        return data()[pos];
    }

    MANGO_STRING_CONSTEXPR reference at(size_type pos) {
        if (pos >= size()) {
            throw std::out_of_range("MangoString::at: position out of range");
        }
        return (*this)[pos];
    }

    MANGO_STRING_CONSTEXPR const_reference at(size_type pos) const {
        if (pos >= size()) {
            throw std::out_of_range("MangoString::at: position out of range");
        }
        return (*this)[pos];
    }

    MANGO_STRING_CONSTEXPR reference front() {
        if (empty()) throw std::out_of_range("MangoString::front: empty string");
        return (*this)[0];
    }

    MANGO_STRING_CONSTEXPR const_reference front() const {
        if (empty()) throw std::out_of_range("MangoString::front: empty string");
        return (*this)[0];
    }

    MANGO_STRING_CONSTEXPR reference back() {
        if (empty()) throw std::out_of_range("MangoString::back: empty string");
        return (*this)[size() - 1];
    }

    MANGO_STRING_CONSTEXPR const_reference back() const {
        if (empty()) throw std::out_of_range("MangoString::back: empty string");
        return (*this)[size() - 1];
    }

    MANGO_STRING_CONSTEXPR pointer data() noexcept { return storage_.data(); }
    MANGO_STRING_CONSTEXPR const_pointer data() const noexcept { return storage_.data(); }
    MANGO_STRING_CONSTEXPR const_pointer c_str() const noexcept { return data(); }

    MANGO_STRING_CONSTEXPR operator view_type() const noexcept {
        return view_type(data(), size());
    }

    // 迭代器
    MANGO_STRING_CONSTEXPR iterator begin() noexcept { return data(); }
    MANGO_STRING_CONSTEXPR const_iterator begin() const noexcept { return data(); }
    MANGO_STRING_CONSTEXPR const_iterator cbegin() const noexcept { return data(); }
    MANGO_STRING_CONSTEXPR iterator end() noexcept { return data() + size(); }
    MANGO_STRING_CONSTEXPR const_iterator end() const noexcept { return data() + size(); }
    MANGO_STRING_CONSTEXPR const_iterator cend() const noexcept { return data() + size(); }

    MANGO_STRING_CONSTEXPR reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    MANGO_STRING_CONSTEXPR const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    MANGO_STRING_CONSTEXPR const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
    MANGO_STRING_CONSTEXPR reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    MANGO_STRING_CONSTEXPR const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    MANGO_STRING_CONSTEXPR const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

    // 修改器
    MANGO_STRING_CONSTEXPR void clear() noexcept {
        if (!empty()) {
            storage_.set_size(0);
            data()[0] = CharT();
        }
    }

    // 统一赋值方法
    template<typename T>
        requires std::constructible_from<view_type, T> ||
    std::is_same_v<std::remove_cvref_t<T>, MangoString>
        MANGO_STRING_CONSTEXPR MangoString& assign(T&& value) {
        using RawT = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<RawT, MangoString>) {
            if (this != &value) {
                assign_impl(value.data(), value.size());
            }
        }
        else {
            view_type sv(value);
            assign_impl(sv.data(), sv.size());
        }
        return *this;
    }

    // 带长度的assign
    template<typename T>
        requires std::convertible_to<T, const CharT*>
    MANGO_STRING_CONSTEXPR MangoString& assign(T&& data, size_type count) {
        return assign_impl(static_cast<const CharT*>(data), count);
    }

    // 填充assign
    MANGO_STRING_CONSTEXPR MangoString& assign(size_type count, CharT ch) {
        if (count == 0) {
            clear();
        }
        else {
            storage_.allocate(count, [&](CharT* dest, size_type c) {
                Traits::assign(dest, c, ch);
                });
        }
        return *this;
    }

    // 统一追加操作
    template<typename T>
        requires std::constructible_from<view_type, T> ||
    std::is_same_v<std::remove_cvref_t<T>, MangoString> ||
        std::is_same_v<std::remove_cvref_t<T>, CharT>
        MANGO_STRING_CONSTEXPR MangoString& operator+=(T&& value) {
        return append(std::forward<T>(value));
    }

    // 统一的append方法
    template<typename T>
        requires std::constructible_from<view_type, T> ||
    std::is_same_v<std::remove_cvref_t<T>, MangoString> ||
        std::is_same_v<std::remove_cvref_t<T>, CharT>
        MANGO_STRING_CONSTEXPR MangoString& append(T&& value) {
        using RawT = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<RawT, MangoString>) {
            return append_impl(value.data(), value.size());
        }
        else if constexpr (std::is_same_v<RawT, CharT>) {
            push_back(value);
            return *this;
        }
        else if constexpr (requires { view_type(value); }) {
            view_type sv(value);
            return append_impl(sv.data(), sv.size());
        }
        return *this;
    }

    // 带长度的append
    template<typename T>
        requires std::convertible_to<T, const CharT*>
    MANGO_STRING_CONSTEXPR MangoString& append(T&& data, size_type count) {
        return append_impl(static_cast<const CharT*>(data), count);
    }

    // 填充append
    MANGO_STRING_CONSTEXPR MangoString& append(size_type count, CharT ch) {
        return append_fill_impl(count, ch);
    }

    // 迭代器append
    template<typename InputIt>
    MangoString& append(InputIt first, InputIt last) {
        for (; first != last; ++first) {
            push_back(*first);
        }
        return *this;
    }

    void push_back(CharT ch) {
        const size_type current_size = size();
        if (current_size == capacity()) {
            reserve(std::max(size_type(16), current_size * 2));
        }
        data()[current_size] = ch;
        data()[current_size + 1] = CharT();
        storage_.set_size(current_size + 1);
    }

    void pop_back() {
        if (empty()) {
            throw std::out_of_range("MangoString::pop_back: empty string");
        }
        const size_type new_size = size() - 1;
        data()[new_size] = CharT();
        storage_.set_size(new_size);
    }

    // 字符串操作
    MANGO_STRING_CONSTEXPR MangoString substr(size_type pos = 0, size_type count = npos) const {
        if (pos > size()) {
            throw std::out_of_range("MangoString::substr: position out of range");
        }
        const size_type len = size() - pos;
        const size_type actual_count = (count == npos) ? len : std::min(count, len);
        return MangoString(data() + pos, actual_count);
    }

    // 统一的find方法
    template<typename T>
        requires std::constructible_from<view_type, T> ||
    std::is_same_v<std::remove_cvref_t<T>, CharT>
        MANGO_STRING_CONSTEXPR size_type find(T&& what, size_type pos = 0) const noexcept {
        using RawT = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<RawT, CharT>) {
            return find_char_impl(what, pos);
        }
        else {
            view_type sv(what);
            return find_string_impl(sv.data(), pos, sv.size());
        }
    }

    // 比较操作符
    MANGO_STRING_CONSTEXPR auto operator<=>(const MangoString& other) const noexcept {
        return view_type(*this) <=> view_type(other);
    }

    MANGO_STRING_CONSTEXPR bool operator==(const MangoString& other) const noexcept {
        if (size() != other.size()) return false;
        return Traits::compare(data(), other.data(), size()) == 0;
    }

    template<typename T>
        requires std::convertible_to<const T&, view_type>
    MANGO_STRING_CONSTEXPR bool operator==(const T& other) const noexcept {
        view_type sv = other;
        if (size() != sv.size()) return false;
        return Traits::compare(data(), sv.data(), size()) == 0;
    }

    // 格式化
    template<typename... Args>
    static MangoString format(view_type fmt, Args&&... args) {
        if constexpr (std::is_same_v<CharT, char>) {
            return MangoString(std::vformat(
                std::string_view(fmt.data(), fmt.size()),
                std::make_format_args(std::forward<Args>(args)...)
            ));
        }
        else if constexpr (std::is_same_v<CharT, wchar_t>) {
            return MangoString(std::vformat(
                std::wstring_view(fmt.data(), fmt.size()),
                std::make_wformat_args(std::forward<Args>(args)...)
            ));
        }
        else {
            std::basic_ostringstream<CharT, Traits> oss;
            ((oss << std::forward<Args>(args)), ...);
            return MangoString(oss.str());
        }
    }

    // 交换
    MANGO_STRING_CONSTEXPR void swap(MangoString& other) noexcept {
        storage_.swap(other.storage_);
    }

    // 转换
    std::basic_string<CharT, Traits> to_std_string() const {
        return std::basic_string<CharT, Traits>(data(), size());
    }

#ifdef MANGO_STRING_SFML_SUPPORT
    operator sf::String() const {
        if constexpr (std::is_same_v<CharT, char>) {
            return sf::String::fromUtf8(begin(), end());
        }
        else if constexpr (std::is_same_v<CharT, wchar_t>) {
            return sf::String(data());
        }
        else if constexpr (std::is_same_v<CharT, char32_t>) {
            return sf::String(data());
        }
        else {
            std::u32string utf32;
            utf32.reserve(size());
            for (size_type i = 0; i < size(); ++i) {
                utf32.push_back(static_cast<char32_t>(data()[i]));
            }
            return sf::String(utf32.data());
        }
    }
#endif

    // 调试支持
    void debug_dump() const {
        std::cout << "MangoString Debug:\n";
        std::cout << "  Size: " << size() << "\n";
        std::cout << "  Capacity: " << capacity() << "\n";
        std::cout << "  SSO: " << (capacity() <= Storage::sso_capacity() ? "Yes" : "No") << "\n";
        std::cout << "  Content: '";
        for (size_type i = 0; i < size(); ++i) {
            std::cout << data()[i];
        }
        std::cout << "'\n";
    }
};

// 非成员函数
template<typename CharT, typename Traits>
void swap(MangoString<CharT, Traits>& lhs, MangoString<CharT, Traits>& rhs) noexcept {
    lhs.swap(rhs);
}

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    const MangoString<CharT, Traits>& str) {
    return os.write(str.data(), str.size());
}

template<typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    MangoString<CharT, Traits>& str) {
    std::basic_string<CharT, Traits> temp;
    is >> temp;
    str.assign(temp);
    return is;
}

// 哈希支持
namespace std {
    template<typename CharT, typename Traits>
    struct hash<MangoString<CharT, Traits>> {
        size_t operator()(const MangoString<CharT, Traits>& str) const noexcept {
            return hash<std::basic_string_view<CharT, Traits>>{}(
                std::basic_string_view<CharT, Traits>(str.data(), str.size()));
        }
    };
}

// 用户定义字面量
inline namespace literals {
    inline String operator""_ms(const char* str, size_t len) {
        return String(str, len);
    }

    inline WString operator""_mws(const wchar_t* str, size_t len) {
        return WString(str, len);
    }
}
