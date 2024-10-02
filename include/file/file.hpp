#include <fstream>
#include <memory>
#include <sstream>
#include <cstring>
class File
{
private:
    void enable_rw()
    {
        // 仅在文件存在的情况下尝试更改权限
        if (access(filename.c_str(), F_OK) == 0)
        {
            mode_t m_mode = 0777;
            if (chmod(filename.c_str(), m_mode) == -1)
            {
                logger.write(LogLevel::ERROR, filename + " Cannot chmod to make it modifiable. Error: " + std::strerror(errno));
            }
        }
        else
        {
            logger.write(LogLevel::ERROR, filename + " does not exist. Cannot change permissions.");
        }
    }

    std::unique_ptr<std::fstream> file_stream;
    std::string filename;
    Log &logger;

public:
    std::string get_filename() const
    {
        return filename;
    }
    void read_Line(std::string *buffer, int line = 1)
    {
        enable_rw();
        if (!file_stream || !file_stream->is_open())
        {
            logger.write(LogLevel::ERROR, "File stream is not open.");
            return;
        }
    
        file_stream->clear(); // Clear any error flags
        file_stream->seekg(0, std::ios::beg); // Move to the beginning of the file
    
        std::string temp;
        int current_line = 1;
    
        while (std::getline(*file_stream, temp))
        {
            if (current_line == line)
            {
                *buffer = temp;
                return;
            }
            current_line++;
        }
    
        logger.write(LogLevel::ERROR, "Specified line number exceeds the total number of lines in the file.");
    }

    template <typename T>
    T get_value(int Line = 1)
    {
        std::string buffer;
        read_Line(&buffer,Line);
        std::stringstream ss(buffer);
        T value;
        ss >> value;
        return value;
    }
    void trunc_write(const std::string buffer)
    {
        enable_rw();
        if (!file_stream || !file_stream->is_open())
        {
            logger.write(LogLevel::ERROR, "File stream is not open.");
            return;
        }
        truncate(filename.c_str(), 0);
        file_stream->seekp(0, std::ios::beg);
        file_stream->write(buffer.c_str(), buffer.size());
        file_stream->flush();
    }
    File(const std::string Filename, Log &logger) : logger(logger)
    {
        filename = Filename;
        enable_rw();
        file_stream = std::make_unique<std::fstream>(Filename, std::ios::in | std::ios::out | std::ios::app);
        if (!file_stream || !file_stream->is_open())
        {
            logger.write(LogLevel::ERROR, "Failed to open file: " + Filename);
            return;
        }
    }
    ~File()
    {
        if (file_stream && file_stream->is_open())
        {
            file_stream->close();
        }
    }
};