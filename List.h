#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>

#define ListDefine(type)                                                                     \
    typedef struct List_##type                                                               \
    {                                                                                        \
        type *data;                                                                          \
        size_t capacity;                                                                     \
        size_t length;                                                                       \
    } List_##type;                                                                           \
                                                                                             \
    inline struct List_##type ListNew_##type(size_t capacity)                                \
    {                                                                                        \
        struct List_##type list = (struct List_##type){                                      \
            .data = malloc(capacity * sizeof(type)),                                         \
            .capacity = capacity,                                                            \
            .length = 0,                                                                     \
        };                                                                                   \
                                                                                             \
        assert(list.data);                                                                   \
                                                                                             \
        return list;                                                                         \
    }                                                                                        \
                                                                                             \
    inline void ListReset_##type(struct List_##type *list)                                   \
    {                                                                                        \
        list->length = 0;                                                                    \
    }                                                                                        \
                                                                                             \
    inline void ListPush_##type(struct List_##type *list, type value)                        \
    {                                                                                        \
        if (list->length >= list->capacity)                                                  \
        {                                                                                    \
            list->capacity *= 2;                                                             \
            list->data = realloc(list->data, list->capacity * sizeof(type));                 \
            assert(list->data);                                                              \
        }                                                                                    \
                                                                                             \
        list->data[list->length] = value;                                                    \
        ++list->length;                                                                      \
    }                                                                                        \
                                                                                             \
    inline type ListPop_##type(struct List_##type *list)                                     \
    {                                                                                        \
        assert(list->length > 0);                                                            \
                                                                                             \
        --list->length;                                                                      \
        return list->data[list->length];                                                     \
    }                                                                                        \
                                                                                             \
    /* Replace the ith element with the last element. Fast, but changes the list's order. */ \
    inline void ListRemoveUnordered_##type(struct List_##type *list, size_t i)               \
    {                                                                                        \
        assert(list->length > i);                                                            \
                                                                                             \
        --list->length;                                                                      \
        list->data[i] = list->data[list->length];                                            \
    }                                                                                        \
                                                                                             \
    inline void ListDelete_##type(struct List_##type *list)                                  \
    {                                                                                        \
        free(list->data);                                                                    \
    }