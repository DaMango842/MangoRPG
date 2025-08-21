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

// 检测SFML是否可用
#if defined(MANGO_STRING_ENABLE_SFML) || __has_include(<SFML/System/String.hpp>)
#include <SFML/System/String.hpp>
#define MANGO_STRING_SFML_SUPPORT 1
#endif

// 主模板声明
template <typename CharT, typename Traits = std::char_traits<CharT>>
class MangoString;

// 类型别名
using String = MangoString<char>;
using WString = MangoString<wchar_t>;
using U8String = MangoString<char8_t>;
using U16String = MangoString<char16_t>;
using U32String = MangoString<char32_t>;

template <typename CharT, typename Traits>
class MangoString {
private:
    // 存储结构 - 使用64位容量
    struct HeapStorage {
        CharT* data;
        uint64_t size;
        uint64_t capacity;
    };

    // SSO 配置 - 使用更可靠的计算方法
    static constexpr size_t SSO_CAPACITY =
        (sizeof(HeapStorage) - 1) / sizeof(CharT);

    // 联合存储
    union {
        CharT sso[SSO_CAPACITY + 1]; // +1 for null terminator
        HeapStorage heap;
    };

    // 使用位域标记SSO状态和大小
    struct Control {
        bool is_sso : 1;
        uint8_t sso_size : 7; // 7 bits for size (max 127)
    } control;

    // 辅助函数
    void destroy() noexcept {
        if (!control.is_sso) {
            delete[] heap.data;
            // 重置堆指针，避免悬空指针
            heap.data = nullptr;
            heap.size = 0;
            heap.capacity = 0;
        }
    }

    void reserve_impl(uint64_t new_cap) {
        // 确保新容量至少为 SSO_CAPACITY
        new_cap = std::max(new_cap, static_cast<uint64_t>(SSO_CAPACITY));

        // 检查容量是否合理
        if (new_cap > std::numeric_limits<uint64_t>::max() / sizeof(CharT)) {
            throw std::length_error("MangoString reserve would exceed maximum size");
        }

        CharT* new_data = new CharT[new_cap + 1];

        const uint64_t current_size = size();
        if (current_size > 0) {
            Traits::copy(new_data, data(), current_size);
        }
        new_data[current_size] = CharT();

        destroy();

        heap.data = new_data;
        heap.size = current_size;
        heap.capacity = new_cap;
        control.is_sso = false;
    }

    void set_size(uint64_t new_size) noexcept {
        if (control.is_sso) {
            // 确保不会超过 SSO 容量
            control.sso_size = static_cast<uint8_t>(std::min(new_size, static_cast<uint64_t>(SSO_CAPACITY)));
        }
        else {
            heap.size = new_size;
        }
    }

    void set_sso_data(const CharT* str, uint64_t len) noexcept {
        // 确保长度不超过 SSO 容量
        len = std::min(len, static_cast<uint64_t>(SSO_CAPACITY));
        Traits::copy(sso, str, len);
        sso[len] = CharT();
        control.sso_size = static_cast<uint8_t>(len);
        control.is_sso = true;
    }

    // 通用分配函数
    template <typename Func>
    void allocate_and_set(uint64_t count, Func setter) {
        if (count <= SSO_CAPACITY) {
            if (!control.is_sso) {
                destroy();
                control.is_sso = true;
            }
            setter(sso, count);
            sso[count] = CharT();
            control.sso_size = static_cast<uint8_t>(count);
        }
        else {
            if (control.is_sso || count > capacity()) {
                destroy();
                heap.data = new CharT[count + 1];
                heap.capacity = count;
                control.is_sso = false;
            }
            setter(heap.data, count);
            heap.data[count] = CharT();
            heap.size = count;
        }
    }

public:
    // 类型别名
    using value_type = CharT;
    using traits_type = Traits;
    using size_type = uint64_t;
    using difference_type = int64_t;
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

    // 构造函数
    constexpr MangoString() noexcept : control{ true, 0 } {
        sso[0] = CharT();
    }

    MangoString(const CharT* str) : control{ true, 0 } {
        if (!str) {
            sso[0] = CharT();
            return;
        }

        const uint64_t len = Traits::length(str);
        allocate_and_set(len, [&](CharT* dest, uint64_t count) {
            Traits::copy(dest, str, count);
            });
    }

    MangoString(const CharT* data, uint64_t count) : control{ true, 0 } {
        if (count == 0) {
            sso[0] = CharT();
            return;
        }

        allocate_and_set(count, [&](CharT* dest, uint64_t count) {
            Traits::copy(dest, data, count);
            });
    }

    MangoString(uint64_t count, CharT ch) : control{ true, 0 } {
        if (count == 0) {
            sso[0] = CharT();
            return;
        }

        allocate_and_set(count, [&](CharT* dest, uint64_t count) {
            Traits::assign(dest, count, ch);
            });
    }

    MangoString(const MangoString& other) : control{ true, 0 } {
        if (other.control.is_sso) {
            Traits::copy(sso, other.sso, other.control.sso_size + 1);
            control.is_sso = true;
            control.sso_size = other.control.sso_size;
        }
        else {
            // 分配新内存，不要直接复制指针！
            heap.data = new CharT[other.heap.capacity + 1];
            Traits::copy(heap.data, other.heap.data, other.heap.size + 1);
            heap.size = other.heap.size;
            heap.capacity = other.heap.capacity;
            control.is_sso = false;
        }
    }

    MangoString(MangoString&& other) noexcept : control{ true, 0 } {
        if (other.control.is_sso) {
            Traits::copy(sso, other.sso, other.control.sso_size + 1);
            control.is_sso = true;
            control.sso_size = other.control.sso_size;
            // 清空源对象
            other.control.sso_size = 0;
            other.sso[0] = CharT();
        }
        else {
            // 转移堆数据所有权
            heap = other.heap;
            control.is_sso = false;

            // 使源对象处于有效的SSO状态
            other.control.is_sso = true;
            other.control.sso_size = 0;
            other.sso[0] = CharT();
        }
    }

    MangoString(std::initializer_list<CharT> ilist)
        : MangoString(ilist.begin(), ilist.size()) {
    }

    template <typename InputIt>
    MangoString(InputIt first, InputIt last) : control{ true, 0 } {
        sso[0] = CharT();
        append(first, last);
    }

    MangoString(view_type sv)
        : MangoString(sv.data(), sv.size()) {
    }

    // SFML 支持
#ifdef MANGO_STRING_SFML_SUPPORT
    MangoString(const sf::String& sfStr) {
        if constexpr (std::is_same_v<CharT, char>) {
            // UTF-32 -> UTF-8
            std::string u8;
            u8.reserve(sfStr.getSize() * 4);
            sf::Utf8::fromUtf32(sfStr.begin(), sfStr.end(), std::back_inserter(u8));
            assign(u8.c_str(), u8.size());
        }
        else if constexpr (std::is_same_v<CharT, char8_t>) {
            // UTF-32 -> UTF-8(char8_t)
            std::string u8;
            u8.reserve(sfStr.getSize() * 4);
            sf::Utf8::fromUtf32(sfStr.begin(), sfStr.end(), std::back_inserter(u8));
            assign(reinterpret_cast<const char8_t*>(u8.data()), u8.size());
        }
        else if constexpr (std::is_same_v<CharT, wchar_t>) {
            // 直接拿 wide（Win: UTF-16, *nix: UTF-32）——但只是“同名宽字符”，不是跨平台同码位
            auto w = sfStr.toWideString();
            assign(w.c_str(), w.size());
        }
        else if constexpr (std::is_same_v<CharT, char16_t>) {
            // UTF-32 -> UTF-16
            std::u16string u16;
            u16.reserve(sfStr.getSize() * 2);
            sf::Utf16::fromUtf32(sfStr.begin(), sfStr.end(), std::back_inserter(u16));
            assign(u16.data(), u16.size());
        }
        else if constexpr (std::is_same_v<CharT, char32_t>) {
            // 直接拷 UTF-32 码点
            std::u32string u32;
            u32.reserve(sfStr.getSize());
            for (auto cp : sfStr) u32.push_back(static_cast<char32_t>(cp));
            assign(u32.data(), u32.size());
        }
        else {
            static_assert(!sizeof(CharT), "Unsupported CharT for MangoString<CharT> from sf::String");
        }
    }

    // ---- MangoString 正确转 sf::String（给 sf::Text 用）----
    operator sf::String() const {
        return sf::String::fromUtf8(begin(), end());
    }
#endif

    ~MangoString() {
        destroy();
    }

    // 赋值操作符
    MangoString& operator=(const MangoString& other) {
        if (this != &other) {
            // 先销毁当前数据
            destroy();

            // 重新初始化控制块
            control.is_sso = true;
            control.sso_size = 0;
            sso[0] = CharT();

            // 复制数据
            if (other.control.is_sso) {
                Traits::copy(sso, other.sso, other.control.sso_size + 1);
                control.is_sso = true;
                control.sso_size = other.control.sso_size;
            }
            else {
                heap.data = new CharT[other.heap.capacity + 1];
                Traits::copy(heap.data, other.heap.data, other.heap.size + 1);
                heap.size = other.heap.size;
                heap.capacity = other.heap.capacity;
                control.is_sso = false;
            }
        }
        return *this;
    }

    MangoString& operator=(MangoString&& other) noexcept {
        if (this != &other) {
            destroy();

            if (other.control.is_sso) {
                Traits::copy(sso, other.sso, other.control.sso_size + 1);
                control.is_sso = true;
                control.sso_size = other.control.sso_size;
                // 清空源对象
                other.control.sso_size = 0;
                other.sso[0] = CharT();
            }
            else {
                // 转移堆数据所有权
                heap = other.heap;
                control.is_sso = false;

                // 使源对象处于有效的SSO状态
                other.control.is_sso = true;
                other.control.sso_size = 0;
                other.sso[0] = CharT();
            }
        }
        return *this;
    }

    MangoString& operator=(const CharT* str) {
        return assign(str);
    }

    MangoString& operator=(CharT ch) {
        assign(1, ch);
        return *this;
    }

    MangoString& operator=(std::initializer_list<CharT> ilist) {
        assign(ilist.begin(), ilist.size());
        return *this;
    }

    MangoString& operator=(view_type sv) {
        assign(sv.data(), sv.size());
        return *this;
    }

    // SFML 赋值支持
#ifdef MANGO_STRING_SFML_SUPPORT
    MangoString& operator=(const sf::String& sfStr) {
        return *this = MangoString(sfStr);
    }
#endif

    // 容量相关
    uint64_t size() const noexcept {
        // 移除不必要的nullptr检查，this永远不会为nullptr
        // if (this == nullptr) { return 0; } // 删除这行

        if (control.is_sso) {
            return static_cast<uint64_t>(control.sso_size);
        }
        else {
            // 确保堆数据有效
            if (heap.data == nullptr) {
                return 0; // 如果data为nullptr，返回0
            }

            // 添加额外的边界检查
            if (heap.size > heap.capacity) {
                // 大小不能超过容量
                return heap.capacity;
            }

            return heap.size;
        }
    }

    uint64_t length() const noexcept {
        return size();
    }

    uint64_t capacity() const noexcept {
        return control.is_sso ? SSO_CAPACITY : heap.capacity;
    }

    bool empty() const noexcept {
        return size() == 0;
    }

    void reserve(uint64_t new_cap) {
        if (new_cap <= capacity()) return;

        if (control.is_sso) {
            reserve_impl(new_cap);
        }
        else {
            if (new_cap > heap.capacity) {
                reserve_impl(new_cap);
            }
        }
    }

    void shrink_to_fit() {
        if (control.is_sso || size() == heap.capacity) return;

        if (size() <= SSO_CAPACITY) {
            CharT temp[SSO_CAPACITY + 1];
            Traits::copy(temp, heap.data, size() + 1);
            delete[] heap.data;
            Traits::copy(sso, temp, size() + 1);
            control.is_sso = true;
            control.sso_size = static_cast<uint8_t>(size());
        }
        else {
            CharT* new_data = new CharT[size() + 1];
            Traits::copy(new_data, heap.data, size() + 1);
            delete[] heap.data;
            heap.data = new_data;
            heap.capacity = size();
        }
    }

    // 元素访问
    CharT& operator[](uint64_t pos) noexcept {
        return data()[pos];
    }

    const CharT& operator[](uint64_t pos) const noexcept {
        return data()[pos];
    }

    CharT& at(uint64_t pos) {
        if (pos >= size()) {
            throw std::out_of_range("MangoString::at");
        }
        return (*this)[pos];
    }

    const CharT& at(uint64_t pos) const {
        if (pos >= size()) {
            throw std::out_of_range("MangoString::at");
        }
        return (*this)[pos];
    }

    CharT& front() noexcept {
        return (*this)[0];
    }

    const CharT& front() const noexcept {
        return (*this)[0];
    }

    CharT& back() noexcept {
        return (*this)[size() - 1];
    }

    const CharT& back() const noexcept {
        return (*this)[size() - 1];
    }

    CharT* data() noexcept {
        return control.is_sso ? sso : heap.data;
    }

    const CharT* data() const noexcept {
        return control.is_sso ? sso : heap.data;
    }

    const CharT* c_str() const noexcept {
        return data();
    }

    operator view_type() const noexcept {
        return view_type(data(), size());
    }

    // 迭代器
    iterator begin() noexcept { return data(); }
    const_iterator begin() const noexcept { return data(); }
    const_iterator cbegin() const noexcept { return data(); }

    iterator end() noexcept { return data() + size(); }
    const_iterator end() const noexcept { return data() + size(); }
    const_iterator cend() const noexcept { return data() + size(); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

    // 修改操作
    void clear() noexcept {
        if (control.is_sso) {
            control.sso_size = 0;
            sso[0] = CharT();
        }
        else {
            heap.size = 0;
            heap.data[0] = CharT();
        }
    }

    MangoString& assign(const CharT* str, uint64_t count) {
        if (count == 0) {
            clear();
            return *this;
        }

        // 自赋值检查
        if (str >= data() && str < data() + size()) {
            return assign(MangoString(str, count));
        }

        allocate_and_set(count, [&](CharT* dest, uint64_t count) {
            Traits::copy(dest, str, count);
            });
        return *this;
    }

    MangoString& assign(const CharT* str) {
        return assign(str, Traits::length(str));
    }

    MangoString& assign(const MangoString& str) {
        if (this == &str) return *this;
        return assign(str.data(), str.size());
    }

    MangoString& assign(uint64_t count, CharT ch) {
        if (count == 0) {
            clear();
            return *this;
        }

        allocate_and_set(count, [&](CharT* dest, uint64_t count) {
            Traits::assign(dest, count, ch);
            });
        return *this;
    }

    MangoString& operator+=(const MangoString& other) {
        return append(other);
    }

    MangoString& operator+=(const CharT* str) {
        return append(str);
    }

    MangoString& operator+=(CharT ch) {
        push_back(ch);
        return *this;
    }

    MangoString& operator+=(std::initializer_list<CharT> ilist) {
        return append(ilist);
    }

    MangoString& operator+=(view_type sv) {
        return append(sv);
    }

    void push_back(CharT ch) {
        const uint64_t current_size = size();
        if (current_size == capacity()) {
            reserve(capacity() * 2);
        }

        CharT* ptr = data();
        ptr[current_size] = ch;
        ptr[current_size + 1] = CharT();
        set_size(current_size + 1);
    }

    void pop_back() noexcept {
        if (size() == 0) return;

        CharT* ptr = data();
        ptr[size() - 1] = CharT();
        set_size(size() - 1);
    }

    MangoString& append(const CharT* data, uint64_t count) {
        if (count == 0) return *this;

        // 处理自追加
        if (data >= this->data() && data < this->data() + size()) {
            return append(MangoString(data, count));
        }

        const uint64_t current_size = size();
        // 添加溢出检查
        if (count > std::numeric_limits<uint64_t>::max() - current_size) {
            throw std::length_error("MangoString append would exceed maximum size");
        }

        const uint64_t new_size = current_size + count;

        if (new_size > capacity()) {
            reserve(std::max(new_size, capacity() * 2));
        }

        CharT* ptr = this->data();
        Traits::copy(ptr + current_size, data, count);
        ptr[new_size] = CharT();
        set_size(new_size);

        return *this;
    }

    MangoString& append(const MangoString& other) {
        return append(other.data(), other.size());
    }

    MangoString& append(const CharT* str) {
        return append(str, Traits::length(str));
    }

    MangoString& append(uint64_t count, CharT ch) {
        const uint64_t current_size = size();
        const uint64_t new_size = current_size + count;

        if (new_size > capacity()) {
            reserve(std::max(new_size, capacity() * 2));
        }

        CharT* ptr = data();
        Traits::assign(ptr + current_size, count, ch);
        ptr[new_size] = CharT();
        set_size(new_size);

        return *this;
    }

    template <typename InputIt>
    MangoString& append(InputIt first, InputIt last) {
        for (; first != last; ++first) {
            push_back(*first);
        }
        return *this;
    }

    MangoString& append(std::initializer_list<CharT> ilist) {
        return append(ilist.begin(), ilist.end());
    }

    MangoString& append(view_type sv) {
        return append(sv.data(), sv.size());
    }

    // 字符串操作
    MangoString substr(uint64_t pos = 0, uint64_t count = npos) const {
        const uint64_t len = size();
        if (pos > len) {
            throw std::out_of_range("MangoString::substr");
        }

        const uint64_t actual_count = (count == npos) ? len - pos : std::min(count, len - pos);
        return MangoString(data() + pos, actual_count);
    }

    uint64_t find(CharT ch, uint64_t pos = 0) const noexcept {
        for (uint64_t i = pos; i < size(); ++i) {
            if (Traits::eq((*this)[i], ch)) {
                return i;
            }
        }
        return npos;
    }

    uint64_t find(const CharT* str, uint64_t pos, uint64_t count) const noexcept {
        if (count == 0) return pos <= size() ? pos : npos;

        const uint64_t len = size();
        if (pos + count > len) return npos;

        for (uint64_t i = pos; i <= len - count; ++i) {
            if (Traits::compare(data() + i, str, count) == 0) {
                return i;
            }
        }
        return npos;
    }

    uint64_t find(const CharT* str, uint64_t pos = 0) const noexcept {
        return find(str, pos, Traits::length(str));
    }

    uint64_t find(const MangoString& str, uint64_t pos = 0) const noexcept {
        return find(str.data(), pos, str.size());
    }

    uint64_t find(view_type sv, uint64_t pos = 0) const noexcept {
        return find(sv.data(), pos, sv.size());
    }

    // 比较操作符
    auto operator<=>(const MangoString& other) const noexcept {
        return view_type(*this) <=> view_type(other);
    }

    bool operator==(const MangoString& other) const noexcept {
        return size() == other.size() &&
            Traits::compare(data(), other.data(), size()) == 0;
    }

    // 格式化函数 (C++20 std::format风格)
    template <typename... Args>
    static MangoString format(view_type fmt, Args&&... args) {
        try {
            // 使用std::format作为底层实现
            if constexpr (std::is_same_v<CharT, char>) {
                std::string result = std::vformat(
                    std::string_view(fmt.data(), fmt.size()),
                    std::make_format_args(std::forward<Args>(args)...)
                );
                return MangoString(result.c_str(), result.size());
            }
            else if constexpr (std::is_same_v<CharT, wchar_t>) {
                std::wstring result = std::vformat(
                    std::wstring_view(fmt.data(), fmt.size()),
                    std::make_wformat_args(std::forward<Args>(args)...)
                );
                return MangoString(result.c_str(), result.size());
            }
            else {
                // 对于其他字符类型，使用字符串流作为备选方案
                std::basic_ostringstream<CharT, Traits> oss;
                oss << fmt;

                // 使用传统方法展开参数包，避免折叠表达式
                using expander = int[];
                (void)expander {
                    0, (oss << std::forward<Args>(args), 0)...
                };

                return MangoString(oss.str().c_str(), oss.str().size());
            }
        }
        catch (const std::exception& e) {
            // 对于所有异常类型，返回错误信息
            if constexpr (std::is_same_v<CharT, char>) {
                return MangoString(std::string("Format error: ") + e.what());
            }
            else if constexpr (std::is_same_v<CharT, wchar_t>) {
                // 将错误信息转换为宽字符串
                std::wstring wmsg = L"Format error: ";
                std::string msg = e.what();
                wmsg += std::wstring(msg.begin(), msg.end());
                return MangoString(wmsg.c_str(), wmsg.size());
            }
            else {
                // 对于其他字符类型，返回简单的错误消息
                return MangoString("Format error");
            }
        }
    }

    // 交换
    void swap(MangoString& other) noexcept {
        if (this == &other) return;

        // 使用手动交换而不是memcpy
        if (control.is_sso && other.control.is_sso) {
            // 两个都是SSO，交换SSO数据
            CharT temp[SSO_CAPACITY + 1]{};
            Traits::copy(temp, sso, SSO_CAPACITY + 1);
            Traits::copy(sso, other.sso, SSO_CAPACITY + 1);
            Traits::copy(other.sso, temp, SSO_CAPACITY + 1);

            std::swap(control, other.control);
        }
        else if (!control.is_sso && !other.control.is_sso) {
            // 两个都是堆分配，交换堆数据
            std::swap(heap, other.heap);
            std::swap(control, other.control);
        }
        else {
            // 一个SSO，一个堆分配，需要转换
            MangoString temp(std::move(*this));
            *this = std::move(other);
            other = std::move(temp);
        }
    }

    // 转换函数
    std::basic_string<CharT, Traits> to_std_string() const {
        return std::basic_string<CharT, Traits>(data(), size());
    }

    // SFML 转换支持
#ifdef MANGO_STRING_SFML_SUPPORT
    sf::String to_sf_string() const { return static_cast<sf::String>(*this); }
#endif

#ifdef MANGO_STRING_DEBUG
    void debug_info() const {
        std::cout << "MangoString Debug Info:\n";
        std::cout << "  is_sso: " << control.is_sso << "\n";
        std::cout << "  size: " << size() << "\n";
        std::cout << "  capacity: " << capacity() << "\n";
        std::cout << "  SSO_CAPACITY: " << SSO_CAPACITY << "\n";
        if (control.is_sso) {
            std::cout << "  sso_size: " << static_cast<int>(control.sso_size) << "\n";
            std::cout << "  sso data: ";
            for (size_t i = 0; i <= SSO_CAPACITY; ++i) {
                std::cout << static_cast<int>(sso[i]) << " ";
            }
            std::cout << "\n";
        }
        else {
            std::cout << "  heap.data: " << static_cast<void*>(heap.data) << "\n";
            std::cout << "  heap.size: " << heap.size << "\n";
            std::cout << "  heap.capacity: " << heap.capacity << "\n";
        }
        std::cout << "  content: " << data() << "\n";
    }
#endif
};

// 非成员函数
template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    const MangoString<CharT, Traits>& str)
{
    return os << str.data();
}

template <typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    MangoString<CharT, Traits>& str)
{
    std::basic_string<CharT, Traits> temp;
    is >> temp;
    str = MangoString<CharT, Traits>(temp.c_str(), temp.size());
    return is;
}

// 哈希支持
namespace std {
    template <typename CharT, typename Traits>
    struct hash<MangoString<CharT, Traits>> {
        size_t operator()(const MangoString<CharT, Traits>& str) const noexcept {
            return hash<std::basic_string_view<CharT, Traits>>{}(
                std::basic_string_view<CharT, Traits>(str.data(), str.size()));
        }
    };
}
