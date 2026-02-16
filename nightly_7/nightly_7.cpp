/*
写一个“自动关闭文件/串口句柄”的RAII类
*/
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <cerrno>
#include <system_error>

class AutoCloseFd{
    public:
        explicit AutoCloseFd(int fd = -1) : fd_(fd) {}
        ~AutoCloseFd() {if(fd_ >= 0) :: close(fd_);}

        AutoCloseFd(const AutoCloseFd&) = delete;
        AutoCloseFd& operator = (const AutoCloseFd) = delete;

        AutoCloseFd(AutoCloseFd && other) noexcept : fd_(other.fd_){other.fd_ = -1;}


};

