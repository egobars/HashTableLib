#pragma once

#include <forward_list>
#include <vector>

template <typename KeyType, typename ValueType, typename HashType = std::hash<KeyType>,
          typename EqualType = std::equal_to<KeyType>>
class HashTable {
public:
    class HashTableIterator {
    public:
        HashTableIterator(HashTable& hash_table, size_t hash_table_index,
                          typename std::forward_list<std::pair<KeyType, ValueType>>::iterator iterator)
            : hash_table_(hash_table) {
            hash_table_index_ = hash_table_index;
            iterator_ = iterator;
        }

        std::pair<KeyType, ValueType> operator*() const {
            return std::make_pair(iterator_->first, iterator_->second);
        }

        typename std::forward_list<std::pair<KeyType, ValueType>>::iterator operator->() const {
            return iterator_;
        }

        HashTableIterator& operator++() {
            while (*this != hash_table_.end() && iterator_ == hash_table_.hash_table_[hash_table_index_].end()) {
                ++hash_table_index_;
                iterator_ = hash_table_.hash_table_[hash_table_index_].begin();
            }
            if (*this != hash_table_.end()) {
                ++iterator_;
            }
            return *this;
        }

        bool operator!=(const HashTableIterator& other) const {
            return iterator_ != other.iterator_;
        }

    private:
        HashTable& hash_table_;
        size_t hash_table_index_ = 0;
        typename std::forward_list<std::pair<KeyType, ValueType>>::iterator iterator_;
    };

    class HashTableIteratorConst {
    public:
        HashTableIteratorConst(const HashTable& hash_table, const size_t hash_table_index,
                               const typename std::forward_list<std::pair<KeyType, ValueType>>::const_iterator iterator)
            : hash_table_(hash_table) {
            hash_table_index_ = hash_table_index;
            iterator_ = iterator;
        }

        std::pair<KeyType, ValueType> operator*() const {
            return make_pair(iterator_->first, iterator_->second);
        }

        HashTableIteratorConst& operator++() {
            while (*this != hash_table_.end() && iterator_ == hash_table_.hash_table_[hash_table_index_].end()) {
                ++hash_table_index_;
                iterator_ = hash_table_.hash_table_[hash_table_index_].begin();
            }
            if (*this != hash_table_.end()) {
                ++iterator_;
            }
            return *this;
        }

        bool operator!=(const HashTableIteratorConst& other) const {
            return iterator_ != other.iterator_;
        }

    private:
        const HashTable& hash_table_;
        size_t hash_table_index_;
        typename std::forward_list<std::pair<KeyType, ValueType>>::const_iterator iterator_;
    };

    HashTable() {
        hash_table_.resize(bucket_count_);
    }

    ValueType& operator[](const KeyType& key) {
        ++size_;
        if (size_ > bucket_count_) {
            Rehash();
        }
        size_t now_hash = hash_(key) % bucket_count_;
        auto now_iterator = hash_table_[now_hash].begin();
        for (std::pair<KeyType, ValueType> cell : hash_table_[now_hash]) {
            if (key_equal_(key, cell.first)) {
                return now_iterator->second;
            }
            ++now_iterator;
        }
        hash_table_[now_hash].push_front({key, 0});
        return hash_table_[now_hash].begin()->second;
    }

    ValueType at(const KeyType& key) const {
        size_t now_hash = hash_(key) % bucket_count_;
        auto now_iterator = hash_table_[now_hash].begin();
        for (std::pair<KeyType, ValueType> cell : hash_table_[now_hash]) {
            if (key_equal_(key, cell.first)) {
                return now_iterator->second;
            }
            ++now_iterator;
        }
        throw std::out_of_range("This element doesn't exist");
    }

    std::pair<HashTableIterator, bool> insert(const std::pair<KeyType, ValueType>& kvp) {
        ++size_;
        if (size_ > bucket_count_) {
            Rehash();
        }
        size_t now_hash = hash_(kvp.first) % bucket_count_;
        auto now_iterator = hash_table_[now_hash].begin();
        for (auto& [key, value] : hash_table_[now_hash]) {
            if (key_equal_(kvp.first, key) && value == kvp.second) {
                return {HashTableIterator(*this, now_hash, now_iterator), false};
            }
            ++now_iterator;
        }
        hash_table_[now_hash].push_front(kvp);
        return {HashTableIterator(*this, now_hash, hash_table_[now_hash].begin()), true};
    }

    std::pair<HashTableIterator, bool> emplace(const KeyType& key, const ValueType& value) {
        std::pair<KeyType, ValueType> kvp = {key, value};
        return insert(kvp);
    }

    bool empty() {
        return size_ == 0;
    }

    size_t size() {
        return size_;
    }

    void clear() {
        hash_table_.clear();
        size_ = 0;
        bucket_count_ = 1000;
        hash_table_.resize(bucket_count_);
    }

    HashTableIterator begin() {
        return HashTableIterator(*this, 0, hash_table_[0].begin());
    }

    HashTableIterator end() {
        return HashTableIterator(*this, bucket_count_ - 1, hash_table_[bucket_count_ - 1].end());
    }

    HashTableIteratorConst begin() const {
        HashTableIteratorConst iterator = HashTableIteratorConst(*this, 0, hash_table_[0].begin());
        return iterator;
    }

    HashTableIteratorConst end() const {
        return HashTableIteratorConst(*this, bucket_count_ - 1, hash_table_[bucket_count_ - 1].begin());
    }

    HashTableIterator find(const KeyType& to_find) {
        size_t now_hash = hash_(to_find) % bucket_count_;
        auto now_iterator = hash_table_[now_hash].begin();
        for (auto& [key, value] : hash_table_[now_hash]) {
            if (key_equal_(to_find, key)) {
                return HashTableIterator(*this, now_hash, now_iterator);
            }
            ++now_iterator;
        }
        return end();
    }

    HashTable(const std::vector<std::pair<KeyType, ValueType>>& vect) {
        for (const auto& [key, value] : vect) {
            this->operator[](key) = value;
        }
    }

    HashTable(const HashTable& hash_table) {
        this->bucket_count_ = hash_table.bucket_count_;
        this->size_ = hash_table.size_;
        this->hash_ = hash_table.hash_;
        this->key_equal_ = hash_table.key_equal_;
        this->hash_table_ = hash_table.hash_table_;
    }

    HashTable& operator=(const HashTable& hash_table) {
        if (this == &hash_table) {
            return *this;
        }
        this->bucket_count_ = hash_table.bucket_count_;
        this->size_ = hash_table.size_;
        this->hash_ = hash_table.hash_;
        this->key_equal_ = hash_table.key_equal_;
        this->hash_table_ = hash_table.hash_table_;
    }

    HashTable(HashTable&& hash_table) {
        this->bucket_count_ = hash_table.bucket_count_;
        this->size_ = hash_table.size_;
        this->hash_ = hash_table.hash_;
        this->key_equal_ = hash_table.key_equal_;
        this->hash_table_ = hash_table.hash_table_;
    }

    HashTable& operator=(const HashTable&& hash_table) {
        if (this == &hash_table) {
            return *this;
        }
        this->bucket_count_ = hash_table.bucket_count_;
        this->size_ = hash_table.size_;
        this->hash_ = hash_table.hash_;
        this->key_equal_ = hash_table.key_equal_;
        this->hash_table_ = hash_table.hash_table_;
    }

    std::vector<std::forward_list<std::pair<KeyType, ValueType>>> hash_table_;

private:
    void Rehash() {
        bucket_count_ *= 2;
        std::vector<std::forward_list<std::pair<KeyType, ValueType>>> new_hash_table(bucket_count_);
        for (auto fl : hash_table_) {
            for (const auto& [key, value] : fl) {
                new_hash_table[hash_(key) % bucket_count_].push_front({key, value});
            }
        }
        hash_table_ = new_hash_table;
    }

    size_t bucket_count_ = 1000;
    size_t size_ = 0;
    HashType hash_;
    EqualType key_equal_;
};
