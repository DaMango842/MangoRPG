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

#if __cplusplus >= 202002L
#define MANGO_STRING_CONSTEXPR constexpr
#else
#define MANGO_STRING_CONSTEXPR
#endif

#if defined(MANGO_STRING_ENABLE_SFML) || __has_include(<SFML/System/String.hpp>)
#include <SFML/System/String.hpp>
#define MANGO_STRING_SFML_SUPPORT 1
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
    static constexpr size_type max_size_limit = std::numeric_limits<size_type>::max() / 2 - 1;

private:
    struct HeapStorage {
        CharT* data;
        size_type size;
        size_type capacity;
    };

    static constexpr size_t SSO_CAPACITY = (sizeof(HeapStorage) - sizeof(uint8_t) - 1) / sizeof(CharT);

    union Storage {
        struct {
            CharT sso[SSO_CAPACITY + 1];
        } sso_storage;

        HeapStorage heap;

        constexpr Storage() noexcept : sso_storage{} {}
        ~Storage() noexcept {}
    } storage;

    struct Control {
        uint8_t is_sso : 1;
        uint8_t sso_size : 7;
        constexpr Control() noexcept : is_sso(1), sso_size(0) {}
        // The fix: Add a new constructor to allow direct initialization.
        constexpr Control(bool sso, uint8_t size) noexcept : is_sso(sso), sso_size(size) {}
    } control;

    void destroy() noexcept {
        if (!control.is_sso) {
            delete[] storage.heap.data;
            storage.heap.data = nullptr;
            storage.heap.size = 0;
            storage.heap.capacity = 0;
        }
    }

    void steal_resources(MangoString&& other) noexcept {
        if (other.control.is_sso) {
            Traits::copy(storage.sso_storage.sso, other.storage.sso_storage.sso, other.control.sso_size + 1);
            control.is_sso = true;
            control.sso_size = other.control.sso_size;
        }
        else {
            storage.heap = other.storage.heap;
            control.is_sso = false;
        }
        other.control.is_sso = true;
        other.control.sso_size = 0;
        other.storage.sso_storage.sso[0] = CharT();
    }

    template <typename Func>
    void allocate_and_set(size_type count, Func setter) {
        if (count > max_size_limit) {
            throw std::length_error("MangoString: allocation exceeds max_size");
        }

        if (count <= SSO_CAPACITY) {
            if (!control.is_sso) {
                destroy();
                control.is_sso = true;
            }
            setter(storage.sso_storage.sso, count);
            storage.sso_storage.sso[count] = CharT();
            control.sso_size = static_cast<uint8_t>(count);
        }
        else {
            HeapStorage new_heap{};
            new_heap.data = new (std::nothrow) CharT[count + 1];
            if (!new_heap.data) {
                throw std::bad_alloc();
            }
            try {
                setter(new_heap.data, count);
                new_heap.data[count] = CharT();
                new_heap.size = count;
                new_heap.capacity = count;
                destroy();
                storage.heap = new_heap;
                control.is_sso = false;
            }
            catch (...) {
                delete[] new_heap.data;
                throw;
            }
        }
    }

    void set_size(size_type new_size) noexcept {
        if (control.is_sso) {
            control.sso_size = static_cast<uint8_t>(new_size);
        }
        else {
            storage.heap.size = new_size;
        }
    }

    void check_range(size_type pos, const char* msg) const {
        if (pos >= size()) {
            throw std::out_of_range(msg);
        }
    }

    void check_null_pointer(const CharT* ptr, const char* msg) const {
        if (ptr == nullptr) {
            throw std::invalid_argument(msg);
        }
    }

    size_type checked_length(const CharT* str) const {
        check_null_pointer(str, "MangoString: null pointer");
        return Traits::length(str);
    }

public:
    // Constructors
    MANGO_STRING_CONSTEXPR MangoString() noexcept : control(true, 0) {
        storage.sso_storage.sso[0] = CharT();
    }

    MangoString(const MangoString& other) : control(true, 0) {
        if (other.control.is_sso) {
            Traits::copy(storage.sso_storage.sso, other.storage.sso_storage.sso, other.control.sso_size + 1);
            control.is_sso = true;
            control.sso_size = other.control.sso_size;
        }
        else {
            allocate_and_set(other.storage.heap.size, [&](CharT* dest, size_type count) {
                Traits::copy(dest, other.storage.heap.data, count);
                });
        }
    }

    MangoString(MangoString&& other) noexcept : control(true, 0) {
        steal_resources(std::move(other));
    }

    template <typename InputIt>
    MangoString(InputIt first, InputIt last) : MangoString() {
        append(first, last);
    }

    // Fixed: The control member is now initialized correctly
    MangoString(const CharT* str) : MangoString(str, str ? checked_length(str) : 0) {}

    // Fixed: The control member is now initialized correctly
    MangoString(const CharT* data, size_type count) : control(count <= SSO_CAPACITY, static_cast<uint8_t>(count)) {
        if (count == 0) {
            storage.sso_storage.sso[0] = CharT();
        }
        else {
            allocate_and_set(count, [&](CharT* dest, size_type c) {
                Traits::copy(dest, data, c);
                });
        }
    }

    // Fixed: The control member is now initialized correctly
    MangoString(size_type count, CharT ch) : control(count <= SSO_CAPACITY, static_cast<uint8_t>(count)) {
        if (count == 0) {
            storage.sso_storage.sso[0] = CharT();
        }
        else {
            allocate_and_set(count, [&](CharT* dest, size_type c) {
                Traits::assign(dest, c, ch);
                });
        }
    }

    MangoString(std::initializer_list<CharT> ilist) : MangoString(ilist.begin(), ilist.size()) {}

    template <typename T>
        requires std::convertible_to<const T&, view_type>
    MangoString(const T& t) : MangoString(view_type(t)) {}

    MangoString(view_type sv) : MangoString(sv.data(), sv.size()) {}

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
        else if constexpr (std::is_same_v<CharT, char16_t>) {
            std::u16string temp = sfStr.toUtf16();
            assign(temp.data(), temp.size());
        }
        else if constexpr (std::is_same_v<CharT, char32_t>) {
            std::u32string temp = sfStr.toUtf32();
            assign(temp.data(), temp.size());
        }
        else {
            throw std::runtime_error("Unsupported character type for SFML conversion");
        }
    }
#endif

    ~MangoString() {
        destroy();
    }

    // Assignment operators
    MangoString& operator=(const MangoString& other) {
        if (this != &other) {
            MangoString temp(other);
            swap(temp);
        }
        return *this;
    }

    MangoString& operator=(MangoString&& other) noexcept {
        if (this != &other) {
            destroy();
            steal_resources(std::move(other));
        }
        return *this;
    }

    MangoString& operator=(const CharT* str) {
        return assign(str);
    }

    MangoString& operator=(CharT ch) {
        return assign(1, ch);
    }

    MangoString& operator=(std::initializer_list<CharT> ilist) {
        return assign(ilist);
    }

    template <typename T>
        requires std::convertible_to<const T&, view_type>
    MangoString& operator=(const T& t) {
        return assign(t);
    }

    // Capacity
    MANGO_STRING_CONSTEXPR size_type size() const noexcept {
        return control.is_sso ? static_cast<size_type>(control.sso_size) : storage.heap.size;
    }

    MANGO_STRING_CONSTEXPR size_type length() const noexcept { return size(); }
    MANGO_STRING_CONSTEXPR bool empty() const noexcept { return size() == 0; }

    MANGO_STRING_CONSTEXPR size_type capacity() const noexcept {
        return control.is_sso ? SSO_CAPACITY : storage.heap.capacity;
    }

    void reserve(size_type new_cap) {
        if (new_cap <= capacity()) return;
        if (new_cap > max_size_limit) {
            throw std::length_error("MangoString::reserve");
        }
        const size_type current_size = size();
        CharT* new_data = new (std::nothrow) CharT[new_cap + 1];
        if (!new_data) {
            throw std::bad_alloc();
        }
        Traits::copy(new_data, data(), current_size + 1);
        destroy();
        storage.heap.data = new_data;
        storage.heap.size = current_size;
        storage.heap.capacity = new_cap;
        control.is_sso = false;
    }

    void shrink_to_fit() {
        if (control.is_sso || size() == storage.heap.capacity) return;
        if (size() <= SSO_CAPACITY) {
            CharT temp[SSO_CAPACITY + 1]{};
            Traits::copy(temp, storage.heap.data, size() + 1);
            destroy();
            Traits::copy(storage.sso_storage.sso, temp, size() + 1);
            control.is_sso = true;
            control.sso_size = static_cast<uint8_t>(size());
        }
        else {
            CharT* new_data = new CharT[size() + 1];
            Traits::copy(new_data, storage.heap.data, size() + 1);
            destroy();
            storage.heap.data = new_data;
            storage.heap.capacity = size();
        }
    }

    // Element access
    MANGO_STRING_CONSTEXPR reference operator[](size_type pos) noexcept {
        return data()[pos];
    }

    MANGO_STRING_CONSTEXPR const_reference operator[](size_type pos) const noexcept {
        return data()[pos];
    }

    MANGO_STRING_CONSTEXPR reference at(size_type pos) {
        check_range(pos, "MangoString::at");
        return (*this)[pos];
    }

    MANGO_STRING_CONSTEXPR const_reference at(size_type pos) const {
        check_range(pos, "MangoString::at");
        return (*this)[pos];
    }

    MANGO_STRING_CONSTEXPR reference front() noexcept { return (*this)[0]; }
    MANGO_STRING_CONSTEXPR const_reference front() const noexcept { return (*this)[0]; }
    MANGO_STRING_CONSTEXPR reference back() noexcept { return (*this)[size() - 1]; }
    MANGO_STRING_CONSTEXPR const_reference back() const noexcept { return (*this)[size() - 1]; }

    MANGO_STRING_CONSTEXPR pointer data() noexcept {
        return control.is_sso ? storage.sso_storage.sso : storage.heap.data;
    }

    MANGO_STRING_CONSTEXPR const_pointer data() const noexcept {
        return control.is_sso ? storage.sso_storage.sso : storage.heap.data;
    }

    MANGO_STRING_CONSTEXPR const_pointer c_str() const noexcept { return data(); }

    MANGO_STRING_CONSTEXPR operator view_type() const noexcept {
        return view_type(data(), size());
    }

    // Iterators
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

    // Modifiers
    MANGO_STRING_CONSTEXPR void clear() noexcept {
        if (control.is_sso) {
            control.sso_size = 0;
        }
        else {
            storage.heap.size = 0;
        }
        data()[0] = CharT();
    }

    MangoString& assign(const CharT* str, size_type count) {
        if (count == 0) {
            clear();
            return *this;
        }
        check_null_pointer(str, "MangoString::assign");
        if (str >= data() && str < data() + size()) {
            return assign(MangoString(str, count));
        }
        allocate_and_set(count, [&](CharT* dest, size_type c) {
            Traits::copy(dest, str, c);
            });
        return *this;
    }

    MangoString& assign(const CharT* str) {
        return assign(str, checked_length(str));
    }

    MangoString& assign(const MangoString& str) {
        if (this != &str) {
            assign(str.data(), str.size());
        }
        return *this;
    }

    MangoString& assign(size_type count, CharT ch) {
        if (count == 0) {
            clear();
            return *this;
        }
        allocate_and_set(count, [&](CharT* dest, size_type c) {
            Traits::assign(dest, c, ch);
            });
        return *this;
    }

    template <typename InputIt>
    MangoString& assign(InputIt first, InputIt last) {
        clear();
        return append(first, last);
    }

    MangoString& assign(std::initializer_list<CharT> ilist) {
        return assign(ilist.begin(), ilist.end());
    }

    template <typename T>
        requires std::convertible_to<const T&, view_type>
    MangoString& assign(const T& t) {
        return assign(view_type(t));
    }

    // Append operations
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

    template <typename T>
        requires std::convertible_to<const T&, view_type>
    MangoString& operator+=(const T& t) {
        view_type sv = t;
        return append(sv.data(), sv.size());
    }

    void push_back(CharT ch) {
        const size_type current_size = size();
        if (current_size == capacity()) {
            reserve(capacity() > 0 ? capacity() * 2 : 16);
        }
        data()[current_size] = ch;
        data()[current_size + 1] = CharT();
        set_size(current_size + 1);
    }

    void pop_back() noexcept {
        if (!empty()) {
            data()[size() - 1] = CharT();
            set_size(size() - 1);
        }
    }

    MangoString& append(const CharT* data, size_type count) {
        if (count == 0) return *this;
        check_null_pointer(data, "MangoString::append");
        const size_type current_size = size();
        if (count > max_size_limit - current_size) {
            throw std::length_error("MangoString::append");
        }
        const size_type new_size = current_size + count;
        if (new_size > capacity()) {
            reserve(std::max(new_size, capacity() * 2));
        }
        Traits::copy(this->data() + current_size, data, count);
        this->data()[new_size] = CharT();
        set_size(new_size);
        return *this;
    }

    MangoString& append(const MangoString& other) {
        return append(other.data(), other.size());
    }

    MangoString& append(const CharT* str) {
        return append(str, checked_length(str));
    }

    MangoString& append(size_type count, CharT ch) {
        const size_type current_size = size();
        const size_type new_size = current_size + count;
        if (new_size > capacity()) {
            reserve(std::max(new_size, capacity() * 2));
        }
        Traits::assign(data() + current_size, count, ch);
        data()[new_size] = CharT();
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

    template <typename T>
        requires std::convertible_to<const T&, view_type>
    MangoString& append(const T& t) {
        view_type sv = t;
        return append(sv.data(), sv.size());
    }

    // String operations
    MANGO_STRING_CONSTEXPR MangoString substr(size_type pos = 0, size_type count = npos) const {
        check_range(pos, "MangoString::substr");
        const size_type len = size();
        const size_type actual_count = (count == npos) ? len - pos : std::min(count, len - pos);
        return MangoString(data() + pos, actual_count);
    }

    MANGO_STRING_CONSTEXPR size_type find(CharT ch, size_type pos = 0) const noexcept {
        for (size_type i = pos; i < size(); ++i) {
            if (Traits::eq((*this)[i], ch)) {
                return i;
            }
        }
        return npos;
    }

    MANGO_STRING_CONSTEXPR size_type find(const CharT* str, size_type pos, size_type count) const noexcept {
        if (count == 0) return pos <= size() ? pos : npos;
        if (pos + count > size()) return npos;
        for (size_type i = pos; i <= size() - count; ++i) {
            if (Traits::compare(data() + i, str, count) == 0) {
                return i;
            }
        }
        return npos;
    }

    MANGO_STRING_CONSTEXPR size_type find(const CharT* str, size_type pos = 0) const noexcept {
        return str ? find(str, pos, Traits::length(str)) : npos;
    }

    MANGO_STRING_CONSTEXPR size_type find(const MangoString& str, size_type pos = 0) const noexcept {
        return find(str.data(), pos, str.size());
    }

    template <typename T>
        requires std::convertible_to<const T&, view_type>
    MANGO_STRING_CONSTEXPR size_type find(const T& t, size_type pos = 0) const noexcept {
        view_type sv = t;
        return find(sv.data(), pos, sv.size());
    }

    // Comparison operators
    MANGO_STRING_CONSTEXPR auto operator<=>(const MangoString& other) const noexcept {
        return view_type(*this) <=> view_type(other);
    }

    MANGO_STRING_CONSTEXPR bool operator==(const MangoString& other) const noexcept {
        return view_type(*this) == view_type(other);
    }

    template <typename T>
        requires std::convertible_to<const T&, view_type>
    MANGO_STRING_CONSTEXPR bool operator==(const T& other) const noexcept {
        return view_type(*this) == view_type(other);
    }

    // Formatting
    template <typename... Args>
    static MangoString format(view_type fmt, Args&&... args) {
        if constexpr (std::is_same_v<CharT, char>) {
            return String(std::vformat(std::string_view(fmt.data(), fmt.size()), std::make_format_args(std::forward<Args>(args)...)));
        }
        else if constexpr (std::is_same_v<CharT, wchar_t>) {
            return WString(std::vformat(std::wstring_view(fmt.data(), fmt.size()), std::make_wformat_args(std::forward<Args>(args)...)));
        }
        else {
            std::basic_ostringstream<CharT, Traits> oss;
            (oss << ... << std::forward<Args>(args));
            return MangoString(oss.str());
        }
    }

    // Swap
    void swap(MangoString& other) noexcept {
        if (this == &other) return;
        if (control.is_sso && other.control.is_sso) {
            CharT temp[SSO_CAPACITY + 1];
            Traits::copy(temp, storage.sso_storage.sso, SSO_CAPACITY + 1);
            Traits::copy(storage.sso_storage.sso, other.storage.sso_storage.sso, SSO_CAPACITY + 1);
            Traits::copy(other.storage.sso_storage.sso, temp, SSO_CAPACITY + 1);
            std::swap(control, other.control);
        }
        else if (!control.is_sso && !other.control.is_sso) {
            std::swap(storage.heap, other.storage.heap);
            std::swap(control, other.control);
        }
        else {
            MangoString temp(std::move(*this));
            *this = std::move(other);
            other = std::move(temp);
        }
    }

    // Conversion
    std::basic_string<CharT, Traits> to_std_string() const {
        return std::basic_string<CharT, Traits>(data(), size());
    }

    // SFML conversion
#ifdef MANGO_STRING_SFML_SUPPORT
    operator sf::String() const {
        return sf::String(begin(), end());
    }
#endif

    // Debug support
    void debug_dump() const {
        std::cout << "MangoString Debug:\n";
        std::cout << "  Size: " << size() << "\n";
        std::cout << "  Capacity: " << capacity() << "\n";
        std::cout << "  SSO: " << (control.is_sso ? "Yes" : "No") << "\n";
        if (control.is_sso) {
            std::cout << "  SSO Size: " << static_cast<int>(control.sso_size) << "\n";
        }
        std::cout << "  Content: '" << data() << "'\n";
    }
};

// Non-member functions
template <typename CharT, typename Traits>
void swap(MangoString<CharT, Traits>& lhs, MangoString<CharT, Traits>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    const MangoString<CharT, Traits>& str) {
    return os << str.data();
}

template <typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    MangoString<CharT, Traits>& str) {
    std::basic_string<CharT, Traits> temp;
    is >> temp;
    str = MangoString<CharT, Traits>(temp);
    return is;
}

// Hash support
namespace std {
    template <typename CharT, typename Traits>
    struct hash<MangoString<CharT, Traits>> {
        size_t operator()(const MangoString<CharT, Traits>& str) const noexcept {
            return hash<std::basic_string_view<CharT, Traits>>{}(
                std::basic_string_view<CharT, Traits>(str.data(), str.size()));
        }
    };
}

// User-defined literals
inline namespace literals {
    inline String operator""_ms(const char* str, size_t len) {
        return String(str, len);
    }
    inline WString operator""_mws(const wchar_t* str, size_t len) {
        return WString(str, len);
    }
}
