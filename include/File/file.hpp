#include <fstream>
#include <string>
class FileSys{
    private:
        std::string FileName;
        std::unique_ptr<std::fstream> File;
        Log &logger;
    public:
        FileSys(Log& logger,const std::string filename):logger(logger){
            FileName = filename;
            File = std::make_unique<std::fstream>(filename,std::ios::in | std::ios::out std::ios::app);
            if (!File->is_open()){
                logger.write(LogLevel::ERROR,"You did not open this file:"+filename);
            }
        }
        std::string read_all(std::string *buffer){
            
        }
        ~FileSys(){
            File->close();
        }
};
