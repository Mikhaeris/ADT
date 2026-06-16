/* vector.h — dynamic array
 *
 * usage:
 * in one file:
 *     #define VECTOR_IMPL_CENTRY
 *     #include "<path>/vector.h"
 * in other place just:
 *     #include "<path>/vector.h"
 */

#ifndef VECTOR_H_SENTRY
#define VECTOR_H_SENTRY

#include <stddef.h>

typedef struct {
    size_t size_;
    size_t capacity_;
    size_t type_size_;
} vec_header;

#define VEC_ALIGN     (_Alignof(max_align_t))
#define VEC_HEAD_SIZE (((sizeof(vec_header) + VEC_ALIGN - 1) / VEC_ALIGN) * VEC_ALIGN)
#define VEC_HEAD(V)   ((vec_header *)((char *)(V) - VEC_HEAD_SIZE))

#define vec_init(VEC, TYPE) (vec_init_impl((void **)&(VEC), sizeof(TYPE)))
#define vec_free(VEC)       (vec_free_impl((void **)&(VEC)))

#define vec_size(VEC)      (VEC_HEAD(VEC)->size_)
#define vec_capacity(VEC)  (VEC_HEAD(VEC)->capacity_)
#define vec_type_size(VEC) (VEC_HEAD(VEC)->type_size_)

#define vec_clear(VEC)            (vec_clear_impl((void **)&(VEC)))
#define vec_insert(VEC, POS, VAL) (*(__typeof__(VEC))vec_insert_impl((void **)&(VEC), (POS)) = (VAL))
#define vec_erase(VEC, POS)       (vec_erase_impl((void **)&(VEC), POS))
#define vec_push_back(VEC, VAL)   (*(__typeof__(VEC))vec_push_back_impl((void **)&(VEC)) = (VAL))
#define vec_pop_back(VEC)         (vec_pop_back_impl((void **)&(VEC)))
#define vec_resize(VEC, COUNT)    (vec_resize_impl((void **)&(VEC), (COUNT)))
#define vec_swap(VEC1, VEC2)      (vec_swap_impl((void **)&(VEC1), (void **)&(VEC2)))

void vec_init_impl(void **vec, size_t type_size);
void vec_free_impl(void **vec);

void vec_clear_impl(void **vec);
void *vec_insert_impl(void **vec, size_t pos);
void vec_erase_impl(void **vec, size_t pos);
void *vec_push_back_impl(void **vec);
void vec_pop_back_impl(void **vec);
void vec_resize_impl(void **vec, size_t count);
void vec_swap_impl(void **vec1, void **vec2);

#endif /* VECTOR_H_SENTRY */

#ifdef VECTOR_IMPL_SENTRY

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEF_CAPACITY 8

void vec_init_impl(void **vec, size_t type_size)
{
    if (*vec) {
        return;
    }

    *vec = malloc(VEC_HEAD_SIZE + (DEF_CAPACITY * type_size));
    if (!(*vec)) {
        perror("malloc");
        abort();
    }

    vec_header *head = (vec_header *)(*vec);
    head->size_      = 0;
    head->capacity_  = DEF_CAPACITY;
    head->type_size_ = type_size;

    *vec = (void *)((uint8_t *)*vec + VEC_HEAD_SIZE);
}

void vec_free_impl(void **vec)
{
    if (!vec || !*vec) {
        return;
    }

    vec_header *head = VEC_HEAD((void *)(*vec));
    free(head);
    *vec = NULL;
}

static void vec_realloc(void **vec)
{
    if (!vec || !*vec) {
        return;
    }

    vec_header *head = VEC_HEAD((void *)(*vec));
    if (head->capacity_ > (SIZE_MAX - VEC_HEAD_SIZE) / 2 / head->type_size_) {
        fprintf(stderr, "realloc overflow\n");
        abort();
    }
    size_t new_capacity = head->capacity_ * 2;
    void *tmp           = realloc((void *)head, VEC_HEAD_SIZE + (new_capacity * head->type_size_));
    if (!tmp) {
        perror("realloc");
        abort();
    }

    head            = (vec_header *)tmp;
    head->capacity_ = new_capacity;
    *vec            = (void *)((uint8_t *)tmp + VEC_HEAD_SIZE);
}

void vec_clear_impl(void **vec)
{
    if (!vec || !*vec) {
        return;
    }

    vec_header *head = VEC_HEAD((void *)(*vec));
    head->size_      = 0;
}

void *vec_insert_impl(void **vec, size_t pos)
{
    if (!vec || !*vec) {
        return NULL;
    }

    vec_header *head = VEC_HEAD((void *)(*vec));

    if (pos > head->size_) {
        return NULL;
    }

    if (pos == head->size_) {
        return vec_push_back_impl(vec);
    }

    if (head->size_ == head->capacity_) {
        vec_realloc(vec);
        head = VEC_HEAD((void *)(*vec));
    }

    uint8_t *data = (uint8_t *)*vec;

    void *dst  = data + ((pos + 1) * head->type_size_);
    void *src  = data + (pos * head->type_size_);
    size_t len = (head->size_ - pos) * head->type_size_;
    memmove(dst, src, len);

    head->size_ += 1;

    return src;
}

void vec_erase_impl(void **vec, size_t pos)
{
    if (!vec || !*vec) {
        return;
    }

    vec_header *head = VEC_HEAD((void *)(*vec));

    if (pos >= head->size_) {
        return;
    }

    uint8_t *data = (uint8_t *)*vec;

    void *dst  = data + (pos * head->type_size_);
    void *src  = data + ((pos + 1) * head->type_size_);
    size_t len = (head->size_ - pos - 1) * head->type_size_;
    memmove(dst, src, len);

    head->size_ -= 1;
}

void *vec_push_back_impl(void **vec)
{
    if (!vec || !*vec) {
        return NULL;
    }

    vec_header *head = VEC_HEAD((void *)(*vec));
    if (head->size_ == head->capacity_) {
        vec_realloc(vec);
        head = VEC_HEAD((void *)(*vec));
    }

    void *slot = (void *)((uint8_t *)*vec + ((head->size_) * head->type_size_));
    head->size_ += 1;

    return slot;
}

void vec_pop_back_impl(void **vec)
{
    if (!vec || !*vec) {
        return;
    }

    vec_header *head = VEC_HEAD((void *)(*vec));
    if (head->size_ == 0) {
        return;
    }

    head->size_ -= 1;
}

void vec_resize_impl(void **vec, size_t count)
{
    if (!vec || !*vec) {
        return;
    }

    vec_header *head = VEC_HEAD((void *)(*vec));

    if (count > (SIZE_MAX - VEC_HEAD_SIZE) / 2 / head->type_size_) {
        fprintf(stderr, "realloc overflow\n");
        abort();
    }

    if (count < head->size_) {
        head->size_ = count;
        return;
    }

    if (count > head->capacity_) {
        size_t new_capacity = count * 2;
        void *tmp           = realloc((void *)head, VEC_HEAD_SIZE + (new_capacity * head->type_size_));
        if (!tmp) {
            perror("realloc");
            abort();
        }
        *vec = (void *)((uint8_t *)tmp + VEC_HEAD_SIZE);
        head = VEC_HEAD((void *)(*vec));

        head->capacity_ = new_capacity;
    }

    void *dest = (uint8_t *)(*vec) + (head->size_ * head->type_size_);
    size_t cnt = (count - head->size_) * head->type_size_;
    memset(dest, 0, cnt);

    head->size_ = count;
}

void vec_swap_impl(void **vec1, void **vec2)
{
    if (!vec1 || !*vec1) {
        return;
    }

    if (!vec2 || !*vec2) {
        return;
    }

    void *tmp = *vec1;
    *vec1     = *vec2;
    *vec2     = tmp;
}

#endif /* VECTOR_IMPL_SENTRY */
