/*
 * Copyright (c) 2019 Andrei Odintsov <forestryks1@gmail.com>
 */

#ifndef LIBSBOX_PLAIN_STRING_VECTOR_H
#define LIBSBOX_PLAIN_STRING_VECTOR_H

#include "context_manager.h"
#include "utils.h"
#include "plain_string.h"
#include "plain_vector.h"

#include <sys/types.h>
#include <string>
#include <cstring>

template<size_t MaxItems, size_t MaxTotalSize>
class PlainStringVector {
public:
    PlainStringVector() {
        items_.push_back(nullptr);
    }

    char **get() {
        return items_.data();
    }

    size_t size() {
        return items_.size() - 1;
    }

    const char *operator[](int pos) const {
        return items_[pos];
    }

    void add(const std::string &str) {
        items_.pop_back();
        items_.push_back(data_.c_str() + data_.size());
        items_.push_back(nullptr);
        data_ += str;
        data_ += '\0';
    }

    void clear() {
        items_.clear();
        items_.push_back(nullptr);
        data_.clear();
    }

private:
    PlainVector<char *, MaxItems + 1> items_;
    PlainString<MaxTotalSize + MaxItems> data_;
};

#endif //LIBSBOX_PLAIN_STRING_VECTOR_H
