#pragma once

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ListDefine(type)                                                                                               \
    typedef struct List_##type                                                                                         \
    {                                                                                                                  \
        type *data;                                                                                                    \
        int32_t capacity;                                                                                              \
        int32_t count;                                                                                                 \
    } List_##type;                                                                                                     \
                                                                                                                       \
    inline struct List_##type ListNew_##type(int32_t capacity)                                                         \
    {                                                                                                                  \
        struct List_##type list = (struct List_##type){                                                                \
            .data = NULL,                                                                                              \
            .capacity = capacity,                                                                                      \
            .count = 0,                                                                                                \
        };                                                                                                             \
                                                                                                                       \
        if (capacity > 0)                                                                                              \
        {                                                                                                              \
            list.data = malloc(capacity * sizeof(type));                                                               \
            assert(list.data);                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        return list;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    inline void ListReset_##type(struct List_##type *list)                                                             \
    {                                                                                                                  \
        list->count = 0;                                                                                               \
    }                                                                                                                  \
                                                                                                                       \
    inline void ListPush_##type(struct List_##type *list, type value)                                                  \
    {                                                                                                                  \
        if (list->count >= list->capacity)                                                                             \
        {                                                                                                              \
            list->capacity *= 2;                                                                                       \
            list->data = realloc(list->data, list->capacity * sizeof(type));                                           \
            assert(list->data);                                                                                        \
        }                                                                                                              \
                                                                                                                       \
        list->data[list->count] = value;                                                                               \
        ++list->count;                                                                                                 \
    }                                                                                                                  \
                                                                                                                       \
    inline type ListPop_##type(struct List_##type *list)                                                               \
    {                                                                                                                  \
        assert(list->count > 0);                                                                                       \
                                                                                                                       \
        --list->count;                                                                                                 \
        return list->data[list->count];                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    inline void ListRemove_##type(struct List_##type *list, int32_t i)                                                 \
    {                                                                                                                  \
        assert(list->count > i);                                                                                       \
                                                                                                                       \
        for (; i < list->count - 1; i++)                                                                               \
        {                                                                                                              \
            list->data[i] = list->data[i + 1];                                                                         \
        }                                                                                                              \
                                                                                                                       \
        --list->count;                                                                                                 \
    }                                                                                                                  \
                                                                                                                       \
    /* Replace the ith element with the last element. Fast, but changes the list's order. */                           \
    inline void ListRemoveUnordered_##type(struct List_##type *list, int32_t i)                                        \
    {                                                                                                                  \
        assert(list->count > i);                                                                                       \
                                                                                                                       \
        --list->count;                                                                                                 \
        list->data[i] = list->data[list->count];                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    inline void ListDelete_##type(struct List_##type *list)                                                            \
    {                                                                                                                  \
        free(list->data);                                                                                              \
    }