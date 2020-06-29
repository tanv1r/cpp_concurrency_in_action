#include <future>
#include <iostream>
#include <vector>

template <>
class packaged_task<std::string(std::vector<char>*, int)>
{
    public:
        template <typename Callable>
        explicit packaged_task(Callable && f);
        std::future<std::string> get_future();
        
        // When () operator is used, it calls f with
        // the passed in arguments
        void operator() (std::vector<char>*, int);
};