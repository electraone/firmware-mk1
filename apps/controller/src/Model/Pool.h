/*
* Electra One MIDI Controller Firmware
* See COPYRIGHT file at the top of the source tree.
*
* This product includes software developed by the
* Electra One Project (http://electra.one/).
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.
*/

/**
 * @file Pool.h
 *
 * @brief Storage for the objects of a preset that hands out short
 *        contiguous runs and never moves what it has handed out.
 *
 * A preset's controls and their values used to be one heap block each, and
 * a run of them cost as much in allocator headers and tree nodes as in
 * data. One growing vector for all of them is not the answer either: a
 * vector grows by doubling, and on a 256 KB part the step that asks for
 * one 27 KB block found no free chunk that large and no room to extend the
 * heap, and the preset died of std::bad_alloc with a hundred kilobytes
 * still free in pieces. So this allocates in pages of a couple of
 * kilobytes, which the allocator can always find, and gives out runs from
 * the current page. A run never straddles pages, so it is contiguous, and
 * a page is never moved or freed until clear(), so a pointer into it is
 * good for the life of the preset.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <climits>
#include <new>
#include <utility>
#include <vector>

template <typename T, size_t PageBytes>
class Pool
{
public:
    Pool() : cursor(nullptr), spaceLeft(0), count(0)
    {
    }

    ~Pool()
    {
        clear();
    }

    Pool(const Pool &) = delete;
    Pool &operator=(const Pool &) = delete;

    /* Movable, so that an object holding one - a Device, say - can still be
     * put into a map by assignment. The pages go with it; the source is left
     * empty. */
    Pool(Pool &&other) noexcept
        : pages(std::move(other.pages)),
          cursor(other.cursor),
          spaceLeft(other.spaceLeft),
          count(other.count)
    {
        other.cursor = nullptr;
        other.spaceLeft = 0;
        other.count = 0;
    }

    Pool &operator=(Pool &&other) noexcept
    {
        if (this != &other) {
            clear();
            pages = std::move(other.pages);
            cursor = other.cursor;
            spaceLeft = other.spaceLeft;
            count = other.count;
            other.cursor = nullptr;
            other.spaceLeft = 0;
            other.count = 0;
        }
        return (*this);
    }

    /** Every page holds this many; a run never crosses from one to the
     *  next, and no run may be longer than this. */
    static constexpr size_t perPage =
        (PageBytes / sizeof(T)) > 0 ? (PageBytes / sizeof(T)) : 1;

    /** Names no object; what indexOf() answers for a pointer this did not
     *  hand out. */
    static constexpr size_t noIndex = SIZE_MAX;

    /** A contiguous run of n default-constructed objects, or nullptr when
     *  there is no memory for another page, or when the run would be longer
     *  than a page - which is what keeps every object addressable by a
     *  number, see indexOf(). */
    T *allocate(size_t n)
    {
        if ((n == 0) || (n > perPage)) {
            return (nullptr);
        }
        if (n > spaceLeft) {
            if (!addPage(perPage)) {
                return (nullptr);
            }
        }

        T *run = cursor;

        for (size_t i = 0; i < n; i++) {
            new (cursor + i) T();
        }
        pages.back().used += n;
        cursor += n;
        spaceLeft -= n;
        count += n;
        return (run);
    }

    /**
     * @brief The number of an object, which at() turns back into the object.
     *
     * Page times perPage plus the place in the page: two bytes name anything
     * in a pool of up to 64K objects, where a pointer costs four. Only for
     * pointers this handed out; anything else answers noIndex.
     */
    size_t indexOf(const T *object) const
    {
        for (size_t p = 0; p < pages.size(); p++) {
            const Page &page = pages[p];

            if ((object >= page.objects) && (object < page.objects + page.used)) {
                return ((p * perPage) + (object - page.objects));
            }
        }
        return (noIndex);
    }

    /** The object with a number from indexOf(), or nullptr. */
    T *at(size_t index)
    {
        const size_t p = index / perPage;
        const size_t slot = index % perPage;

        if ((p >= pages.size()) || (slot >= pages[p].used)) {
            return (nullptr);
        }
        return (pages[p].objects + slot);
    }

    const T *at(size_t index) const
    {
        return (const_cast<Pool *>(this)->at(index));
    }

    size_t size(void) const
    {
        return (count);
    }

    /** How many pages have been taken; at() answers for indexes below
     *  pageCount() * perPage, give or take the unused tail of a page. */
    size_t pageCount(void) const
    {
        return (pages.size());
    }

    bool empty(void) const
    {
        return (count == 0);
    }

    /** Destroy every object and give every page back. */
    void clear(void)
    {
        for (auto &page : pages) {
            for (size_t i = 0; i < page.used; i++) {
                page.objects[i].~T();
            }
            ::operator delete(page.objects);
        }
        pages.clear();
        pages.shrink_to_fit();
        cursor = nullptr;
        spaceLeft = 0;
        count = 0;
    }

private:
    struct Page {
        T *objects;
        size_t used;
    };

    bool addPage(size_t capacity)
    {
        T *objects = static_cast<T *>(
            ::operator new(capacity * sizeof(T), std::nothrow));

        if (!objects) {
            return (false);
        }
        pages.push_back(Page{ objects, 0 });
        cursor = objects;
        spaceLeft = capacity;
        return (true);
    }

    std::vector<Page> pages;
    T *cursor;
    size_t spaceLeft;
    size_t count;
};
