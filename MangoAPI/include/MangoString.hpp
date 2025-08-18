#pragma once

#include <cstring>
#include <stdexcept>
#include <vector>
#include <string>
#include <concepts>
#include <sstream>
#include <memory>
#include <string_view>
#include <type_traits>
#include <compare>
#include <format>
#include <iterator>
#include <cuchar>
#include <clocale>
#include <locale>
#include <codecvt>
#include <cctype>
#include <algorithm>

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
    // SSO 配置
    static constexpr size_t SSO_CAPACITY =
        (sizeof(void*) * 3 - 1) / sizeof(CharT);

    // 存储结构
    struct HeapStorage {
        CharT* data;
        size_t size;
        size_t capacity;
    };

    union {
        CharT sso[SSO_CAPACITY + 1]; // +1 for null terminator
        HeapStorage heap;
    };

    bool is_sso : 1;
    unsigned char sso_size : 7; // 7 bits for size (max 127)

    // 辅助函数
    void destroy() noexcept {
        if (!is_sso) {
            delete[] heap.data;
        }
    }

    void reserve_impl(size_t new_cap) {
        new_cap = std::max(new_cap, static_cast<size_t>(SSO_CAPACITY));
        CharT* new_data = new CharT[new_cap + 1];

        const size_t current_size = size();
        if (current_size > 0) {
            Traits::copy(new_data, data(), current_size);
        }
        new_data[current_size] = CharT();

        destroy();

        heap.data = new_data;
        heap.size = current_size;
        heap.capacity = new_cap;
        is_sso = false;
    }

    void set_size(size_t new_size) noexcept {
        if (is_sso) {
            sso_size = static_cast<unsigned char>(new_size);
        }
        else {
            heap.size = new_size;
        }
    }

    void set_sso_data(const CharT* str, size_t len) noexcept {
        Traits::copy(sso, str, len);
        sso[len] = CharT();
        sso_size = static_cast<unsigned char>(len);
        is_sso = true;
    }

public:
    // 类型别名
    using value_type = CharT;
    using traits_type = Traits;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
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
    constexpr MangoString() noexcept : is_sso(true), sso_size(0) {
        sso[0] = CharT();
    }

    MangoString(const CharT* str) {
        if (!str) {
            is_sso = true;
            sso_size = 0;
            sso[0] = CharT();
            return;
        }

        const size_t len = Traits::length(str);
        if (len <= SSO_CAPACITY) {
            set_sso_data(str, len);
        }
        else {
            heap.data = new CharT[len + 1];
            Traits::copy(heap.data, str, len + 1);
            heap.size = len;
            heap.capacity = len;
            is_sso = false;
        }
    }

    MangoString(const CharT* data, size_t count) {
        if (count == 0) {
            is_sso = true;
            sso_size = 0;
            sso[0] = CharT();
            return;
        }

        if (count <= SSO_CAPACITY) {
            set_sso_data(data, count);
        }
        else {
            heap.data = new CharT[count + 1];
            Traits::copy(heap.data, data, count);
            heap.data[count] = CharT();
            heap.size = count;
            heap.capacity = count;
            is_sso = false;
        }
    }

    MangoString(size_t count, CharT ch) {
        if (count <= SSO_CAPACITY) {
            Traits::assign(sso, count, ch);
            sso[count] = CharT();
            is_sso = true;
            sso_size = static_cast<unsigned char>(count);
        }
        else {
            heap.data = new CharT[count + 1];
            Traits::assign(heap.data, count, ch);
            heap.data[count] = CharT();
            heap.size = count;
            heap.capacity = count;
            is_sso = false;
        }
    }

    MangoString(const MangoString& other) {
        if (other.is_sso) {
            Traits::copy(sso, other.sso, other.sso_size + 1);
            is_sso = true;
            sso_size = other.sso_size;
        }
        else {
            heap.data = new CharT[other.heap.capacity + 1];
            Traits::copy(heap.data, other.heap.data, other.heap.size + 1);
            heap.size = other.heap.size;
            heap.capacity = other.heap.capacity;
            is_sso = false;
        }
    }

    MangoString(MangoString&& other) noexcept {
        if (other.is_sso) {
            Traits::copy(sso, other.sso, other.sso_size + 1);
            is_sso = true;
            sso_size = other.sso_size;
        }
        else {
            heap = other.heap;
            is_sso = false;

            // 使other处于有效但空的状态
            other.is_sso = true;
            other.sso_size = 0;
            other.sso[0] = CharT();
        }
    }

    MangoString(std::initializer_list<CharT> ilist)
        : MangoString(ilist.begin(), ilist.size()) {
    }

    template <typename InputIt>
    MangoString(InputIt first, InputIt last) {
        is_sso = true;
        sso_size = 0;
        sso[0] = CharT();

        append(first, last);
    }

    MangoString(view_type sv)
        : MangoString(sv.data(), sv.size()) {
    }

    ~MangoString() {
        destroy();
    }

    // 赋值操作符
    MangoString& operator=(const MangoString& other) {
        if (this != &other) {
            MangoString tmp(other);
            swap(tmp);
        }
        return *this;
    }

    MangoString& operator=(MangoString&& other) noexcept {
        if (this != &other) {
            destroy();

            if (other.is_sso) {
                Traits::copy(sso, other.sso, other.sso_size + 1);
                is_sso = true;
                sso_size = other.sso_size;
            }
            else {
                heap = other.heap;
                is_sso = false;

                other.is_sso = true;
                other.sso_size = 0;
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

    // 容量相关
    size_t size() const noexcept {
        return is_sso ? sso_size : heap.size;
    }

    size_t length() const noexcept {
        return size();
    }

    size_t capacity() const noexcept {
        return is_sso ? SSO_CAPACITY : heap.capacity;
    }

    bool empty() const noexcept {
        return size() == 0;
    }

    void reserve(size_t new_cap) {
        if (new_cap <= capacity()) return;

        if (is_sso) {
            reserve_impl(new_cap);
        }
        else {
            if (new_cap > heap.capacity) {
                reserve_impl(new_cap);
            }
        }
    }

    void shrink_to_fit() {
        if (is_sso || size() == heap.capacity) return;

        if (size() <= SSO_CAPACITY) {
            CharT temp[SSO_CAPACITY + 1];
            Traits::copy(temp, heap.data, size() + 1);
            delete[] heap.data;
            Traits::copy(sso, temp, size() + 1);
            is_sso = true;
            sso_size = static_cast<unsigned char>(size());
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
    CharT& operator[](size_t pos) noexcept {
        return data()[pos];
    }

    const CharT& operator[](size_t pos) const noexcept {
        return data()[pos];
    }

    CharT& at(size_t pos) {
        if (pos >= size()) {
            throw std::out_of_range("MangoString::at");
        }
        return (*this)[pos];
    }

    const CharT& at(size_t pos) const {
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
        return is_sso ? sso : heap.data;
    }

    const CharT* data() const noexcept {
        return is_sso ? sso : heap.data;
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
        if (is_sso) {
            sso_size = 0;
            sso[0] = CharT();
        }
        else {
            heap.size = 0;
            heap.data[0] = CharT();
        }
    }

    MangoString& assign(const CharT* str, size_t count) {
        if (count == 0) {
            clear();
            return *this;
        }

        // 自赋值检查
        if (str >= data() && str < data() + size()) {
            return assign(MangoString(str, count));
        }

        if (count <= SSO_CAPACITY) {
            if (!is_sso) {
                destroy();
                is_sso = true;
            }
            Traits::copy(sso, str, count);
            sso[count] = CharT();
            sso_size = static_cast<unsigned char>(count);
        }
        else {
            if (is_sso || count > capacity()) {
                destroy();
                heap.data = new CharT[count + 1];
                heap.capacity = count;
                is_sso = false;
            }
            Traits::copy(heap.data, str, count);
            heap.data[count] = CharT();
            heap.size = count;
        }
        return *this;
    }

    MangoString& assign(const CharT* str) {
        return assign(str, Traits::length(str));
    }

    MangoString& assign(const MangoString& str) {
        if (this == &str) return *this;
        return assign(str.data(), str.size());
    }

    MangoString& assign(size_t count, CharT ch) {
        if (count == 0) {
            clear();
            return *this;
        }

        if (count <= SSO_CAPACITY) {
            if (!is_sso) {
                destroy();
                is_sso = true;
            }
            Traits::assign(sso, count, ch);
            sso[count] = CharT();
            sso_size = static_cast<unsigned char>(count);
        }
        else {
            if (is_sso || count > capacity()) {
                destroy();
                heap.data = new CharT[count + 1];
                heap.capacity = count;
                is_sso = false;
            }
            Traits::assign(heap.data, count, ch);
            heap.data[count] = CharT();
            heap.size = count;
        }
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
        const size_t current_size = size();
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

    MangoString& append(const CharT* data, size_t count) {
        if (count == 0) return *this;

        // 处理自追加
        if (data >= this->data() && data < this->data() + size()) {
            return append(MangoString(data, count));
        }

        const size_t current_size = size();
        const size_t new_size = current_size + count;

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

    MangoString& append(size_t count, CharT ch) {
        const size_t current_size = size();
        const size_t new_size = current_size + count;

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
    MangoString substr(size_t pos = 0, size_t count = npos) const {
        const size_t len = size();
        if (pos > len) {
            throw std::out_of_range("MangoString::substr");
        }

        const size_t actual_count = (count == npos) ? len - pos : std::min(count, len - pos);
        return MangoString(data() + pos, actual_count);
    }

    size_t find(CharT ch, size_t pos = 0) const noexcept {
        for (size_t i = pos; i < size(); ++i) {
            if (Traits::eq((*this)[i], ch)) {
                return i;
            }
        }
        return npos;
    }

    size_t find(const CharT* str, size_t pos, size_t count) const noexcept {
        if (count == 0) return pos <= size() ? pos : npos;

        const size_t len = size();
        if (pos + count > len) return npos;

        for (size_t i = pos; i <= len - count; ++i) {
            if (Traits::compare(data() + i, str, count) == 0) {
                return i;
            }
        }
        return npos;
    }

    size_t find(const CharT* str, size_t pos = 0) const noexcept {
        return find(str, pos, Traits::length(str));
    }

    size_t find(const MangoString& str, size_t pos = 0) const noexcept {
        return find(str.data(), pos, str.size());
    }

    size_t find(view_type sv, size_t pos = 0) const noexcept {
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
                // 对于其他字符类型，转换为宽字符处理
                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
                std::wstring wfmt = conv.from_bytes(
                    reinterpret_cast<const char*>(fmt.data()),
                    reinterpret_cast<const char*>(fmt.data() + fmt.size())
                );

                std::wstring wresult = std::vformat(
                    wfmt,
                    std::make_wformat_args(std::forward<Args>(args)...)
                );

                std::string u8result = conv.to_bytes(wresult);
                return MangoString(
                    reinterpret_cast<const CharT*>(u8result.c_str()),
                    u8result.size() / sizeof(CharT)
                );
            }
        }
        catch (const std::format_error& e) {
            throw std::runtime_error(std::string("Format error: ") + e.what());
        }
    }

    // 交换
    void swap(MangoString& other) noexcept {
        if (this == &other) return;

        // 使用临时缓冲区交换
        alignas(MangoString) char buffer[sizeof(MangoString)];
        std::memcpy(buffer, this, sizeof(MangoString));
        std::memcpy(this, &other, sizeof(MangoString));
        std::memcpy(&other, buffer, sizeof(MangoString));
    }

    // 转换函数
    std::basic_string<CharT, Traits> to_std_string() const {
        return std::basic_string<CharT, Traits>(data(), size());
    }
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

// 转换函数模板
template <typename T>
concept StringConvertible = requires(T a) {
    { std::to_string(a) } -> std::convertible_to<std::string>;
};

template <typename T>
MangoString<char> to_mango_string(T&& value) {
    if constexpr (std::is_same_v<std::decay_t<T>, const char*> ||
        std::is_same_v<std::decay_t<T>, char*>) {
        return String(value);
    }
    else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        return String(value.c_str(), value.size());
    }
    else if constexpr (std::is_same_v<std::decay_t<T>, String>) {
        return value;
    }
    else if constexpr (StringConvertible<T>) {
        return String(std::to_string(value).c_str());
    }
    else {
        std::ostringstream oss;
        oss << value;
        return String(oss.str().c_str(), oss.str().size());
    }
}

// 宽字符版本
template <typename T>
MangoString<wchar_t> to_wmango_string(T&& value) {
    if constexpr (std::is_same_v<std::decay_t<T>, const wchar_t*> ||
        std::is_same_v<std::decay_t<T>, wchar_t*>) {
        return WString(value);
    }
    else if constexpr (std::is_same_v<std::decay_t<T>, std::wstring>) {
        return WString(value.c_str(), value.size());
    }
    else if constexpr (std::is_same_v<std::decay_t<T>, WString>) {
        return value;
    }
    else if constexpr (StringConvertible<T>) {
        return WString(std::to_wstring(value).c_str());
    }
    else {
        std::wostringstream woss;
        woss << value;
        return WString(woss.str().c_str(), woss.str().size());
    }
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
