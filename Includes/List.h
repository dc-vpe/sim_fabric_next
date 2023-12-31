//
// Created by krw10 on 8/25/2023.
//

#ifndef DSL_CPP_LIST_H
#define DSL_CPP_LIST_H

#include "dsl_types.h"
#include "ErrorProcessing.h"
#include <memory>

/// \desc This template class creates a dynamically sizable array of items. Syntax is similar to
///       the C# list type.
template<class Type>
class List
{
public:
    /// \desc Creates a new list
    List()
    {
        size  = 0;
        count = 0;
        array = nullptr;
    }

    /// \desc Frees the memory used by the list.
    ~List()
    {
        delete []array;
    }

    /// \desc Gets the value at index.
    Type &get(int64_t index)
    {
        Extend(index);
        return array[index];
    }

    /// \desc Sets the type into the list at the index location. The list is extended as needed.
    /// \param index Value where the type should be placed.
    /// \param type Type to place in the list at index.
    /// \return True if successful, false if out of memory.
    bool Set(size_t index, Type type)
    {
        if ( !Extend(index) )
        {
            return false;
        }

        array[index] = type;

        return true;
    }

    /// \desc Access to the list elements, list is extended if necessary.
    Type &operator[](size_t index)
    {
        return get(index);
    }

    /// \desc Peeks at the stack_top of the stack without removing it.
    /// \return Element on the stack_top of the stack.
    Type &peek(int64_t offset)
    {
        return array[count - offset];
    }

    /// \desc Appends a new element to the list.
    bool push_back(Type type)
    {
        if (!Extend(count + 1))
        {
            return false;
        }
        memcpy(array + count, &type, sizeof(Type));
        ++count;
        return true;
    }

    /// \desc Returns the last element of the list and reduces the list count by 1.
    Type &pop_back()
    {
        if ( count == 0)
        {
            Extend(1);
            return array[0];
        }
        --count;
        return array[count];
    }

    /// \desc sets the list to 0 elements, does not resize the list elements.
    void Clear() { count = 0; }

    /// \desc returns the number of active elements in the list.
    int64_t Count()
    {
        return count;
    }

    /// \desc Removes the item at index, all subsequent items are moved downward.
    void Remove(int64_t index)
    {
        for(int64_t ii=index; ii<count; ++ii)
        {
            memcpy(&array[ii], &array[ii+1], size * sizeof(Type));
        }
        count--;
    }

    /// \desc Get the last element in the list.
    [[maybe_unused]] Type &Last() { return array[count-1];}


    /// \desc Sets the end of the list to index.
    /// \param newSize New size of the list.
    void Resize(int64_t newSize)
    {
        if ( newSize >=0 && newSize < count )
        {
            count = newSize;
        }
        else
        {
            Extend(newSize);
        }
    }

    /// \desc Swaps the last two elements.
    /// \remark If the list contains less than two elements it remains unchanged.
    [[maybe_unused]] void swap_back()
    {
        if ( count > 1 )
        {
            Type first = pop_back();
            Type second = pop_back();
            push_back(second);
            push_back(first);
        }
    }

    /// \desc Gets size of the list buffer in elements.
    /// \return The number of allocated elements in the list.
    int64_t Size() { return size; }

    /// \desc Gets a read only pointer to the internal array.
    const Type *Array() { return array; }

    /// \desc Call after manually updating the array to set the count to the correct size.
    /// \param newCount new count.
    void SetCount(int newCount) { count = newCount; }

private:
    /// \desc pointer to the buffer used to store list elements.
    Type *array;

    /// \desc allocated size of the buffer.
    int64_t size;

    /// \desc count of elements in the list.
    int64_t count;

    /// \desc Extends the list.
    bool Extend(int64_t index)
    {
        if (index >= size)
        {
            if ( size > 0 )
            {
                Type *tmp = new Type[index + ALLOC_BLOCK_SIZE];
                if (tmp == nullptr)
                {
                    PrintIssue(2043, "Failed to increase memory for list.", true, true);
                    return false;
                }
                memcpy(tmp, array, size * sizeof(Type));
                size = index + ALLOC_BLOCK_SIZE;
                delete []array;
                array = tmp;
            }
            else
            {
                size = index + ALLOC_BLOCK_SIZE;
                array = new Type[size];
                if (array == nullptr)
                {
                    PrintIssue(2043, "Failed to increase memory for list.", true, true);
                    return false;
                }
            }
        }

        return true;
    }
};

#endif //DSL_CPP_LIST_H
