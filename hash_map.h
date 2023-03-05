#include<iostream>
#include <stdexcept>
#include<initializer_list>
#include<array>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
    struct Node {
        size_t w = 0;
        int8_t state = 0;
        std::pair<const KeyType, ValueType> *p = nullptr;
        Node() = default;

        explicit Node(const std::pair<const KeyType, ValueType> &p_, size_t w_ = 0) : w(w_) {
            p = new std::pair<const KeyType, ValueType>(p_);
        }

        Node &operator=(const Node &node) {
            if (this == &node) {
                return *this;
            }
            delete p;
            p = node.p;
            w = node.w;
            state = node.state;
            return *this;
        }
        bool GetEnd() const {
            return state & 1;
        }
        bool GetState() const {
            return state & 2;
        }
        bool GetDeleted() const {
            return state & 4;
        }
        friend void swap(Node &a, Node &b) {
            std::swap(a.w, b.w);
            std::swap(a.state, b.state);
            std::swap(a.p, b.p);
        }

        ~Node() {
            //std::cout << "node destructor: " << p << "\n";
            delete p;
        }
    };

    class iterator {
    public:
        iterator() : ptr(nullptr) {}

        explicit iterator(Node *it) : ptr(it) {}

        iterator(const iterator &it) : ptr(it.ptr) {}

        iterator &operator=(const iterator &it) {
            if (this == &it) {
                return *this;
            }
            ptr = it.ptr;
            return *this;
        }

        std::pair<const KeyType, ValueType> &operator*() const {
            return *(ptr->p);
        }

        std::pair<const KeyType, ValueType> *operator->() const {
            return ptr->p;
        }

        iterator operator++() {
            if (ptr->GetEnd()) {
                ptr++;
                return *this;
            }
            ptr++;
            while (!ptr->GetState()) {
                if (ptr->GetEnd()) {
                    ptr++;
                    return *this;
                }
                ptr++;
            }
            return *this;
        }

        iterator operator++(int) {
            iterator it(*this);
            if (ptr->GetEnd()) {
                ptr++;
                return it;
            }
            ptr++;
            while (!ptr->GetState()) {
                if (ptr->GetEnd()) {
                    ptr++;
                    return it;
                }
                ptr++;
            }
            return it;
        }

        friend bool operator==(const iterator &a, const iterator &b) {
            return a.ptr == b.ptr;
        }

        friend bool operator!=(const iterator &a, const iterator &b) {
            return a.ptr != b.ptr;
        }

    private:
        Node *ptr;
    };

    class const_iterator {
    public:
        const_iterator() : const_ptr(nullptr) {}

        explicit const_iterator(const Node *it) : const_ptr(it) {}

        const_iterator(const const_iterator &it) : const_ptr(it.const_ptr) {}

        const std::pair<const KeyType, ValueType> &operator*() const {
            return *(const_ptr->p);
        }

        const std::pair<const KeyType, ValueType> *operator->() const {
            return const_ptr->p;
        }

        const_iterator &operator=(const const_iterator &it) {
            if (this == &it) {
                return *this;
            }
            const_ptr = it.const_ptr;
            return *this;
        }

        const_iterator operator++() {
            if (const_ptr->GetEnd()) {
                const_ptr++;
                return *this;
            }
            const_ptr++;
            while (!const_ptr->GetState()) {
                if (const_ptr->GetEnd()) {
                    const_ptr++;
                    return *this;
                }
                const_ptr++;
            }
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator it(*this);
            if (const_ptr->GetEnd()) {
                const_ptr++;
                return it;
            }
            const_ptr++;
            while (!const_ptr->GetState()) {
                if (const_ptr->GetEnd()) {
                    const_ptr++;
                    return it;
                }
                const_ptr++;
            }
            return it;
        }

        friend bool operator==(const const_iterator &a, const const_iterator &b) {
            return a.const_ptr == b.const_ptr;
        }

        friend bool operator!=(const const_iterator &a, const const_iterator &b) {
            return a.const_ptr != b.const_ptr;
        }

    private:
        const Node *const_ptr;
    };

    explicit HashMap(Hash hash_ = Hash()) : table_size(start_size), filled(0), hash(hash_) {
        arr = new Node[table_size];
        arr[table_size - 1].state |= 1;
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType> > list, Hash hash_ = Hash()) : hash(hash_){
        table_size = start_size;
        arr = new Node[table_size];
        arr[table_size - 1].state |= 1;
        filled = 0;
        for (const std::pair<KeyType, ValueType> &p: list) {
            this->insert(p);
        }
    }

    template<class Iterator>
    HashMap(Iterator begin, Iterator end, Hash hash_ = Hash()) : hash(hash_) {
        table_size = start_size;
        filled = 0;
        arr = new Node[table_size];
        arr[table_size - 1].state |= 1;
        while (begin != end) {
            this->insert(*begin);
            begin++;
        }
    }

    HashMap(const HashMap &H) : hash(H.hash) {
        table_size = H.table_size;
        filled = 0;
        arr = new Node[table_size];
        arr[table_size - 1].state |= 1;
        for (size_t h = 0; h < table_size; ++h) {
            if (!(H.arr[h].p))
                continue;
            this->insert(*(H.arr[h].p));
        }
    }

    ~HashMap() {
        delete[] arr;
    }

    HashMap &operator=(const HashMap &H) {
        if (this == &H) {
            return *this;
        }
        delete[] arr;
        table_size = H.table_size;
        filled = 0;
        arr = new Node[table_size];
        arr[table_size - 1].state |= 1;
        hash = H.hash;
        for (size_t h = 0; h < table_size; ++h) {
            if (!(H.arr[h].p))
                continue;
            this->insert(*(H.arr[h].p));
        }
        return *this;
    }

    iterator begin() {
        iterator it = this->end();
        for (size_t i = 0; i < table_size; ++i) {
            if (arr[i].GetState()) {
                iterator it2(&arr[i]);
                it = it2;
                return it;
            }
        }
        return it;
    }

    iterator end() {
        Node *e = &arr[table_size - 1];
        e++;
        iterator it(e);
        return it;
    }

    const_iterator begin() const {
        const_iterator it = this->end();
        for (size_t i = 0; i < table_size; ++i) {
            if (arr[i].GetState()) {
                const_iterator it2(&arr[i]);
                it = it2;
                return it;
            }
        }
        return it;
    }

    const_iterator end() const {
        const Node *e = &arr[table_size - 1];
        e++;
        const_iterator it(e);
        return it;
    }

    void insert(const std::pair<KeyType, ValueType> &p) {
        if (check()) {
            resize();
        }
        if (this->find(p.first) != this->end()) {
            return;
        }
        filled++;
        size_t h = hash(p.first) % table_size;
        Node now(p, 0);
        now.state |= 2;
        if (now.state & 4) {
            now.state ^= 4;
        }
        while (arr[h].GetState()) {
            if (now.state & 1) {
                now.state ^= 1;
            }
            if (arr[h].w < now.w) {
                swap(arr[h], now);
            }
            arr[h].state |= 2;
            if (now.state & 4) {
                now.state ^= 4;
            }
            now.w++;
            h = new_hash(h);
        }
        if (arr[h].p)
            delete arr[h].p;
        arr[h].p = new std::pair<const KeyType, ValueType>(*now.p);
        arr[h].state |= 2;
        if (now.state & 4) {
            now.state ^= 4;
        }
        arr[h].w = now.w;
        arr[table_size - 1].state |= 1;
    }

    iterator find(const KeyType &key) {
        if (filled == 0) {
            return this->end();
        }
        size_t h = hash(key) % table_size;
        size_t steps = 0;
        while ((arr[h].GetState() || arr[h].GetDeleted()) && (!arr[h].p || !((arr[h].p)->first == key))) {
            steps++;
            h = new_hash(h);
            if (steps > table_size) {
                return this->end();
            }
        }
        if (!arr[h].GetState()) {
            return this->end();
        }
        Node *p = (arr + h);
        iterator it(p);
        return it;
    }

    const_iterator find(const KeyType &key) const {
        if (filled == 0) {
            return this->end();
        }
        size_t h = hash(key) % table_size;
        size_t steps = 0;
        while ((arr[h].GetState() || arr[h].GetDeleted()) && (!arr[h].p || !((arr[h].p)->first == key))) {
            steps++;
            h = new_hash(h);
            if (steps > table_size) {
                return this->end();
            }
        }
        if (!arr[h].state) {
            return this->end();
        }
        Node *p = (arr + h);
        const_iterator it(p);
        return it;
    }

    void erase(const KeyType &key) {
        if (filled == 0) {
            return;
        }
        size_t h = hash(key) % table_size;
        size_t steps = 0;
        while ((arr[h].GetState() || arr[h].GetDeleted()) && (!arr[h].p || !((arr[h].p)->first == key))) {
            steps++;
            h = new_hash(h);
            if (steps > table_size) {
                return;
            }
        }
        if (!arr[h].GetState()) {
            return;
        }
        filled--;
        if (arr[h].state & 2) {
            arr[h].state ^= 2;
        }
        arr[h].state |= 4;
    }

    ValueType &operator[](const KeyType &key) {
        if (this->find(key) == this->end()) {
            this->insert({key, ValueType()});
            iterator it = this->find(key);
            return it->second;
        }
        size_t h = hash(key) % table_size;
        while (!(arr[h].p->first == key)) {
            h = new_hash(h);
        }
        return arr[h].p->second;
    }

    const ValueType &at(const KeyType &key) const {
        const_iterator ct = this->find(key);
        const_iterator ce = this->end();
        if (ct == ce) {
            throw std::out_of_range("HashMap: out of range");
        }
        size_t h = hash(key) % table_size;
        while (!(arr[h].p->first == key)) {
            h = new_hash(h);
        }
        return arr[h].p->second;
    }

    void clear() {
        table_size = start_size;
        filled = 0;
        delete[] arr;
        arr = new Node[table_size];
        for (size_t h = 0; h < table_size; ++h) {
            arr[h].state = 0;
        }
        arr[table_size - 1].state |= 1;
    }

    size_t size() const {
        return filled;
    }

    bool empty() const {
        return filled == 0;
    }

    Hash hash_function() const {
        return hash;
    }

    void debug() const {
        for (size_t i = 0; i < table_size; ++i) {
            std::cout << arr[i].state;
        }
        std::cout << "\n";
    }

private:
    Node *arr;
    size_t table_size = 0, filled = 0, step = 5;
    Hash hash;
    constexpr const static double load_factor = 0.75;
    static const size_t start_size = 31;
    constexpr static const std::array<size_t, 19> P = {{
                                                               5, 11, 23, 47, 97, 197, 397, 1597, 6421, 25717,
                                                               51437, 102877, 205759,
                                                               411527, 823117, 1311689, 1611737, 1811759, 1911787
                                                       }};
    constexpr const static std::array<size_t, 5> S = {{19, 5, 11, 7, 13}}; /// {4, 0, 1, 2, 3} mod 5

    size_t new_hash(size_t h) const {
        size_t ans = (h + step >= table_size) ? h + step - table_size : h + step;
        return ans;
    }

    bool check() const {
        return (double) table_size * load_factor <= filled;
    }

    size_t new_size(const size_t sz) {
        for (size_t new_sz: P) {
            if (new_sz > sz) {
                return new_sz;
            }
        }
        return P.back();
    }

    void resize() {
        size_t sz = new_size(table_size);
        step = S[step % S.size()];
        Node *n_arr = new Node[sz];
        std::swap(sz, table_size);
        std::swap(arr, n_arr);
        filled = 0;
        for (size_t h = 0; h < sz; ++h) {
            if (!n_arr[h].p || !n_arr[h].GetState()) {
                continue;
            }
            this->insert(*(n_arr[h].p));
        }
        arr[table_size - 1].state |= 1;
        delete[] n_arr;
    }
};
