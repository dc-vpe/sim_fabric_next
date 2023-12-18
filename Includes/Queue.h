//
// Created by krw10 on 9/10/2023.
//

#ifndef DSL_CPP_QUEUE_H
#define DSL_CPP_QUEUE_H

#include "List.h"

/// \desc very simple queue.
template<class Type>
class Queue
{
public:
    Queue()
    {
        front = 0;
        back = 0;
    }

    ~Queue()
        = default;

    /// \desc Adds a new item to the end of the queue.
    void Enqueue(Type type)
    {
        array[back++] = type;
    }

    /// \desc Returns the item at the front of the queue.
    Type &Dequeue()
    {
        return array[front++];
    }

    /// \desc Checks if the queue is empty.
    /// \return True if the queue is empty, else false.
    bool IsEmpty()
    {
        return front == back;
    }

    /// \desc Resets the queue back to empty state.
    void Clear()
    {
        front = 0;
        back = 0;
    }

    /// \desc Peeks at an element in the queue.
    /// \param offset Offset from the front of the queue, - to look backwards, + to look forwards.
    /// \return The element, the caller needs to make sure that the peeked element is in range.
    Type &Peek(int64_t offset = 0)
    {
        return array[front + offset];
    }

    /// \desc Gets the front or start of the queue.
    /// \return The front index of the queue.
    [[maybe_unused]] int64_t Front() { return front; }

    /// \desc Gets the back or end of the queue.
    /// \return The back index of the queue.
    [[maybe_unused]] int64_t Back() { return back; }

    /// \desc Reads the number of items in the queue.
    /// \return The number of items in the queue.
    int64_t Count() { return back-front; }

    /// \desc Gets the element at the absolute index.
    /// \return Item at the index.
    Type &Get(int64_t index)
    {
        if (index >= 0 && index <= array.Count() )
        {
            return array[index];
        }
        return nullptr;
    }

private:
    List<Type>  array;
    int64_t front;
    int64_t back;
};

#endif //DSL_CPP_QUEUE_H
