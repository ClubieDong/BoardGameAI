/*-
 * Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include <climits>
#include <cstddef>
#include <memory>

template <typename T, size_t BlockSize = 4096>
class StaticMemoryPool
{
public:
    // Member types
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using const_pointer = const T *;
    using const_reference = const T &;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using unique_ptr = std::unique_ptr<value_type, void(*)(pointer)>;

    // Can only allocate one object at a time. n and hint are ignored
    inline static pointer allocate()
    {
        ++count_;
        if (freeSlots_)
        {
            pointer result = reinterpret_cast<pointer>(freeSlots_);
            freeSlots_ = freeSlots_->next;
            return result;
        }
        if (currentSlot_ >= lastSlot_)
            allocateBlock();
        return reinterpret_cast<pointer>(currentSlot_++);
    }
    inline static void deallocate(pointer p)
    {
        if (p)
        {
            --count_;
            reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
            freeSlots_ = reinterpret_cast<slot_pointer_>(p);
        }
    }

    inline static constexpr size_type max_size() noexcept
    {
        constexpr size_type maxBlocks = -1 / BlockSize;
        return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
    }

    template <typename U, typename... Args>
    inline static void construct(U *p, Args &&... args) { new (p) U(std::forward<Args>(args)...); }
    template <typename U>
    inline static void destroy(U *p) { p->~U(); }

    template <typename... Args>
    inline static pointer newElement(Args &&... args)
    {
        pointer result = allocate();
        construct<value_type>(result, std::forward<Args>(args)...);
        return result;
    }
    inline static void deleteElement(pointer p)
    {
        if (p)
        {
            p->~value_type();
            deallocate(p);
        }
    }

    template <typename... Args>
    inline static unique_ptr make_unique(Args &&... args)
    {
        pointer p = newElement(std::forward<Args>(args)...);
        return unique_ptr(p, deleteElement);
    }

    inline static size_t size() { return count_ * sizeof(T); }

private:
    union Slot_
    {
        value_type element;
        Slot_ *next;
    };

    using data_pointer_ = char *;
    using slot_type_ = Slot_;
    using slot_pointer_ = Slot_ *;

    inline static size_t count_ = 0;
    inline static slot_pointer_ currentBlock_ = nullptr;
    inline static slot_pointer_ currentSlot_ = nullptr;
    inline static slot_pointer_ lastSlot_ = nullptr;
    inline static slot_pointer_ freeSlots_ = nullptr;

    inline static size_type padPointer(data_pointer_ p, size_type align) noexcept
    {
        uintptr_t result = reinterpret_cast<uintptr_t>(p);
        return ((align - result) % align);
    }
    inline static void allocateBlock()
    {
        // Allocate space for the new block and store a pointer to the previous one
        data_pointer_ newBlock = reinterpret_cast<data_pointer_>(operator new(BlockSize));
        reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
        currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
        // Pad block body to staisfy the alignment requirements for elements
        data_pointer_ body = newBlock + sizeof(slot_pointer_);
        size_type bodyPadding = padPointer(body, alignof(slot_type_));
        currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
        lastSlot_ = reinterpret_cast<slot_pointer_>(newBlock + BlockSize - sizeof(slot_type_) + 1);
    }

    static_assert(BlockSize >= 2 * sizeof(slot_type_), "BlockSize too small.");
};
