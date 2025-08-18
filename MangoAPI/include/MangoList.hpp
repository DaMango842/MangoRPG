#ifndef MANGOLIST_HPP
#define MANGOLIST_HPP

#include <iterator>
#include <utility>
#include <stdexcept>
#include <initializer_list>
#include <memory>

template <typename T>
class MangoList {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;

        template <typename... Args>
        explicit Node(Args&&... args, Node* p = nullptr, Node* n = nullptr)
            : data(std::forward<Args>(args)...), prev(p), next(n) {
        }
    };

    Node* dummy;    // 哨兵节点
    size_t count;   // 元素计数

public:
    // 迭代器类
    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        explicit iterator(Node* node = nullptr) noexcept : current(node) {}

        reference operator*() const noexcept { return current->data; }
        pointer operator->() const noexcept { return &(current->data); }

        iterator& operator++() noexcept {
            current = current->next;
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator tmp = *this;
            current = current->next;
            return tmp;
        }

        iterator& operator--() noexcept {
            current = current->prev;
            return *this;
        }

        iterator operator--(int) noexcept {
            iterator tmp = *this;
            current = current->prev;
            return tmp;
        }

        bool operator==(const iterator& other) const noexcept {
            return current == other.current;
        }

        bool operator!=(const iterator& other) const noexcept {
            return current != other.current;
        }

        Node* base() const noexcept { return current; }

    private:
        Node* current;
    };

    class const_iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        explicit const_iterator(Node* node = nullptr) noexcept : current(node) {}
        const_iterator(iterator it) noexcept : current(it.base()) {}

        reference operator*() const noexcept { return current->data; }
        pointer operator->() const noexcept { return &(current->data); }

        const_iterator& operator++() noexcept {
            current = current->next;
            return *this;
        }

        const_iterator operator++(int) noexcept {
            const_iterator tmp = *this;
            current = current->next;
            return tmp;
        }

        const_iterator& operator--() noexcept {
            current = current->prev;
            return *this;
        }

        const_iterator operator--(int) noexcept {
            const_iterator tmp = *this;
            current = current->prev;
            return tmp;
        }

        bool operator==(const const_iterator& other) const noexcept {
            return current == other.current;
        }

        bool operator!=(const const_iterator& other) const noexcept {
            return current != other.current;
        }

    private:
        Node* current;
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // 构造函数
    MangoList() noexcept : count(0) {
        dummy = new Node(T());
        dummy->prev = dummy;
        dummy->next = dummy;
    }

    // 初始化列表构造函数
    MangoList(std::initializer_list<T> init) : MangoList() {
        for (const auto& value : init) {
            emplace_back(value);
        }
    }

    // 拷贝构造函数
    MangoList(const MangoList& other) : MangoList() {
        for (const T& val : other) {
            push_back(val);
        }
    }

    // 移动构造函数
    MangoList(MangoList&& other) noexcept : MangoList() {
        swap(other);
    }

    // 析构函数
    ~MangoList() {
        clear();
        delete dummy;
    }

    // 拷贝赋值运算符
    MangoList& operator=(const MangoList& other) {
        if (this != &other) {
            MangoList temp(other);
            swap(temp);
        }
        return *this;
    }

    // 移动赋值运算符
    MangoList& operator=(MangoList&& other) noexcept {
        if (this != &other) {
            clear();
            swap(other);
        }
        return *this;
    }

    // 初始化列表赋值
    MangoList& operator=(std::initializer_list<T> init) {
        MangoList temp(init);
        swap(temp);
        return *this;
    }

    // 交换两个链表
    void swap(MangoList& other) noexcept {
        std::swap(dummy, other.dummy);
        std::swap(count, other.count);
    }

    // 元素访问
    T& front() {
        if (empty()) throw std::out_of_range("List is empty");
        return dummy->next->data;
    }

    const T& front() const {
        if (empty()) throw std::out_of_range("List is empty");
        return dummy->next->data;
    }

    T& back() {
        if (empty()) throw std::out_of_range("List is empty");
        return dummy->prev->data;
    }

    const T& back() const {
        if (empty()) throw std::out_of_range("List is empty");
        return dummy->prev->data;
    }

    // 容量
    [[nodiscard]] bool empty() const noexcept { return count == 0; }
    [[nodiscard]] size_t size() const noexcept { return count; }

    // 修改器
    void push_back(const T& value) {
        emplace_back(value);
    }

    void push_back(T&& value) {
        emplace_back(std::move(value));
    }

    void push_front(const T& value) {
        emplace_front(value);
    }

    void push_front(T&& value) {
        emplace_front(std::move(value));
    }

    void pop_back() {
        if (empty()) throw std::out_of_range("List is empty");
        erase(--end());
    }

    void pop_front() {
        if (empty()) throw std::out_of_range("List is empty");
        erase(begin());
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        Node* pos_node = const_cast<Node*>(pos.base());
        Node* new_node = new Node(std::forward<Args>(args)..., pos_node->prev, pos_node);
        pos_node->prev->next = new_node;
        pos_node->prev = new_node;
        ++count;
        return iterator(new_node);
    }

    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }

    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        Node* new_node = new Node(std::forward<Args>(args)..., dummy->prev, dummy);
        dummy->prev->next = new_node;
        dummy->prev = new_node;
        ++count;
        return new_node->data;
    }

    template <typename... Args>
    reference emplace_front(Args&&... args) {
        Node* new_node = new Node(std::forward<Args>(args)..., dummy, dummy->next);
        dummy->next->prev = new_node;
        dummy->next = new_node;
        ++count;
        return new_node->data;
    }

    iterator erase(const_iterator pos) {
        if (pos == end()) throw std::out_of_range("Cannot erase end iterator");
        Node* pos_node = const_cast<Node*>(pos.base());
        Node* next_node = pos_node->next;
        pos_node->prev->next = pos_node->next;
        pos_node->next->prev = pos_node->prev;
        delete pos_node;
        --count;
        return iterator(next_node);
    }

    void clear() noexcept {
        Node* current = dummy->next;
        while (current != dummy) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        dummy->prev = dummy;
        dummy->next = dummy;
        count = 0;
    }

    // 迭代器
    iterator begin() noexcept { return iterator(dummy->next); }
    const_iterator begin() const noexcept { return const_iterator(dummy->next); }
    const_iterator cbegin() const noexcept { return begin(); }

    iterator end() noexcept { return iterator(dummy); }
    const_iterator end() const noexcept { return const_iterator(dummy); }
    const_iterator cend() const noexcept { return end(); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }

    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const noexcept { return rend(); }
};

// 非成员函数
template <typename T>
void swap(MangoList<T>& lhs, MangoList<T>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif // MANGOLIST_HPP
