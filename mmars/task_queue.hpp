#pragma once

/**
 * \brief task_queue represents a circular buffer that contains the active tasks of a warrior.
 */
class task_queue
{
    int                     _size = 0;
    int                     _front = 0;
    int                     _count = 0;
    std::vector<uint32_t>   _data;

public:
    explicit task_queue(int size)
    {
        _size = size;
        _data = std::vector<uint32_t>(size, 0);
        clear();
    }

    void clear()
    {
        _front = 0;
        _count = 0;
    }

    bool empty() const
    {
        return _count == 0;
    }

    bool full() const
    {
        return _count == _size;
    }

    uint32_t count() const
    {
        return _count;
    }

    bool enqueue(uint32_t val)
    {
        if (full()) return false;

        int end = (_front + _count) % _size;
        _data[end] = val;
        _count++;
        return true;
    }

    uint32_t dequeue()
    {
        if (empty()) return 0;

        uint32_t val = _data[_front];
        _front = _front == _size ? 0 : _front + 1;
        _count--;
        return val;
    }

    uint32_t peek(int i)
    {
        return _data[_front + i % _size];
    }
};