#pragma once

#include <cstdint>
#include <stdint.h>

class DataTemplate
{
    virtual void format(char *buffer, int size) = 0;
};

template <class Data, int HISTORY_SIZE>
class Logger
{
private:
    const char *filename;
    Data buffer[HISTORY_SIZE];
    size_t idx = 0;

public:
    Logger(const char *filename) : filename(filename){};
    ~Logger();
    void log(Data data);
};

template <class Data, int HISTORY_SIZE>
Logger<Data, HISTORY_SIZE>::~Logger()
{
    constexpr int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    FILE *logs = fopen(this->filename, "w");
    for (size_t i = 0; i < this->idx; ++i)
    {
        this->buffer[i].format(buffer, BUFFER_SIZE);
        fprintf(logs, "%s\n", buffer);
    }
    fclose(logs);
}

template <class Data, int HISTORY_SIZE>
void Logger<Data, HISTORY_SIZE>::log(Data data)
{
    if (this->idx == HISTORY_SIZE)
    {
        return;
    }
    this->buffer[this->idx++] = data;
}