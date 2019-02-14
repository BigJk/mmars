#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <future>

/**
 * \brief A generic thread pool to scatter tasks over a fixed number of threads.
 * \tparam T The type that tasks will return
 */
template <typename T>
class thread_pool
{
private:
    typedef std::tuple<std::shared_ptr<std::promise<T>>, std::function<T()>> work_request;

    std::atomic<int>        _work_left;
    std::atomic<bool>       _should_stop;

    std::mutex              _lock;
    std::condition_variable _cv;

    std::mutex                                  _lock_queue;
    std::queue<work_request>                    _work;
    std::vector<std::shared_ptr<std::thread>>   _threads;

    void worker()
    {
        while (true)
        {
            // Wait for work or interrupt
            {
                std::unique_lock lock(_lock);
                _cv.wait(lock, [this]()
                {
                    return _should_stop || _work_left > 0;
                });
            }

            while(_work_left > 0)
            {
                // Get work
                bool has_work = false;;
                work_request request;
                {
                    std::unique_lock queue_lock(_lock_queue);
                    if (!_work.empty())
                    {
                        has_work = true;
                        request = _work.front();
                        _work.pop();
                        --_work_left;
                    }
                }

                // Work found
                if (has_work)
                {
                    request._Myfirst._Val->set_value(request._Get_rest()._Myfirst._Val());
                }
            }

            // Interrupt
            if (_should_stop) return;
        }
    }

public:
    explicit thread_pool(int thread_count)
    {
        _work_left = 0;
        _should_stop = false;
        for (int i = 0; i < thread_count; ++i)
        {
            _threads.push_back(std::make_shared<std::thread>([this]
            {
                this->worker();
            }));
        }
    }

    /**
     * \brief Shuts the thread pool down. This will process left over work and stops and joins all threads.
     * After calling this function the pool is not usable anymore.
     */
    void shutdown()
    {
        if (_should_stop) throw std::exception("thread pool is already closed");

        _should_stop = true;
        _cv.notify_all();

        for (auto && thread : _threads)
        {
            if(thread->joinable())
                thread->join();
        }

        _threads.clear();
    }

    /**
     * \brief Inserts work into the task queue.
     * \param func The function that should be executed
     * \return Returns a future object with the result
     */
    std::future<T> enqueue_work(std::function<T()> func)
    {
        if (_should_stop) throw std::exception("thread pool is already closed");

        auto promise = std::make_shared<std::promise<T>>();

        {
            std::unique_lock lock(_lock_queue);
            _work.push(std::make_tuple(promise, func));
            ++_work_left;
            _cv.notify_one();
        }

        return promise->get_future();
    }

    /**
     * \brief Checks if the pool is closed.
     * \return True if the pool is closed
     */
    bool is_closed() const
    {
        return _should_stop;
    }
};
