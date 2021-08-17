# pragma once

#include <functional>
#include <deque>

struct DeletionQueue
{
    // stores deletion/destruction functions
    std::deque<std::function<void()>> deletors;
    
    // add functions to queue
    void PushFunction(std::function<void()>&& func)
    {
        deletors.push_back(func);
    }

    // Reverse iterate thorugh queue and execute function
    void Flush()
    {
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
            (*it)();
        deletors.clear();
    }
};