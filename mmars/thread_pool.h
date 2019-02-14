#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <future>

template <typename T>
class thread_queue
{
private:
    typedef std::tuple < std::shared_ptr<std::promise<T>>, std::function<T()>> work_request;

    std::atomic<int>        _work_left;
    std::atomic<bool>       _should_stop;
    std::mutex              _lock;
    std::condition_variable _cv;

    std::mutex _lock_queue;
    std::queue<work_request> _work;
    std::vector<std::shared_ptr<std::thread>> _threads;

    void work()
    {
        while (true)
        {
            // Wait for work or interrupt
            std::unique_lock lock(_lock);
            _cv.wait(lock, [_should_stop, _work_left]()
            {
                return _should_stop || _work_left > 0;
            });

            // Interrupt
            if (_should_stop) return;

            // Get work
            bool has_work = false;;
            work_request request;

            _lock_queue.lock();
            if (!_work.empty())
            {
                has_work = true;
                request = _work.front();
                _work.pop();
                --_work_left;
            }
            _lock_queue.unlock();

            // Work found
            if(has_work)
            {
                request._Myfirst._Val->set_value(request._Get_rest()._Myfirst._Val());
            }
        }
    }

public:
    explicit thread_queue(int thread_count)
    {
        for (int i = 0; i < thread_count; ++i)
        {
            _threads.push_back(std::make_shared<std::thread>(&thread_queue::work, this));
        }
        _should_stop = false;
    }

    void join()
    {
        _should_stop = true;

        std::unique_lock lock(_lock);
        _cv.wait(lock, [_work_left]()
        {
            return _work_left == 0;
        });

        for (auto && thread : _threads)
        {
            thread->join();
        }
    }

    std::shared_ptr<std::promise<T>> enqueue_work(std::function<T()> func)
    {
        if (_should_stop) throw std::exception("thread pool is already closing");

        auto promise = std::make_shared<std::promise<T>>();

        _lock_queue.lock();
        _work.push(std::make_tuple(promise, func));
        _lock_queue.unlock();

        return promise;
    }
};
