#ifndef DSL_STACK_H
#define DSL_STACK_H

#include "dsl_types.h"
#include <malloc.h>
#include <memory.h>
#include <cstring>
#include <cstdio>
#include "List.h"

template<class Type>
class Stack
{
public:
    /// \desc default constructor sets the stack to ALLOC_BLOCK_SIZE elements.
    Stack()
    {
        stack_top = 0;
    }

    /// \desc Pushes the Type onto the stack_top of the stack.
    void push_back(Type &type)
    {
        array[stack_top++] = type;
    }

    /// \desc Reduces the top of the stack by one element.
    void reduce()
    {
        if ( stack_top > 0 )
        {
            --stack_top;
        }
    }

    /// \desc Removes and returns the stack_top item on the stack.
    /// \return The element on the stack_top of the stack.
    /// \remark Exception is thrown if the stack is empty.
    Type &pop_back()
    {
        if ( stack_top > 0 )
        {
            return array[--stack_top];
        }
        return array[0];
    }

    /// \desc Peeks at the stack_top of the stack without removing it.
    /// \return Element on the stack_top of the stack.
    Type &peek(int64_t offset)
    {
        return array[stack_top - offset];
    }

    /// \desc Gets a pointer to the stack_top at the specified index.
    Type &operator[](int64_t index)
    {
        return array[index];
    }

    /// \desc Value of the stack_top of the stack if 0 the stack is empty.
    [[nodiscard]] int64_t top() const
    {
        return stack_top;
    }

    /// \desc Sets the stack_top of the stack to 0.
    void clear()
    {
        stack_top = 0;
        array.Clear();
    }

    /// \desc Gets the number of elements in the stack.
    /// \return The number of elements in the stack.
    int64_t count() { return stack_top; }
private:
    List<Type> array;
    int64_t stack_top;
};

#endif