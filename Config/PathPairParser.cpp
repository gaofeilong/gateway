#include <iostream>
#include <stdio.h>
#include "Utils/Log/Log.h"
#include "Config/PathPairParser.h"

using std::map;
using std::string;
using std::vector;

PathPairParser::PathPairParser(const string& pathPairFile):m_PathPairFile(pathPairFile)
{
}

PathPairParser::~PathPairParser()
{
}

int PathPairParser::Write(const std::map<std::string, std::string>& pathMap)
{
        FILE* stream = fopen(m_PathPairFile.c_str(), "w");          
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                return -1;
        }

        map<string, string>::const_iterator mssIter = pathMap.begin();
        fprintf(stream, "[ArchivePath]\n");
        for ( ; mssIter != pathMap.end(); ++mssIter) {
                fprintf(stream, "%s %s\n", mssIter->first.c_str(), mssIter->second.c_str());
        }

        if (fclose(stream) != 0) {
                LOG_ERROR("close stream error"); 
        }
        return 0;
}

int PathPairParser::Read(std::map<std::string, std::string>& pathMap)
{
        if (access(m_PathPairFile.c_str(), F_OK) != 0) { //文件不存在
                return 0; 
        }

        FILE* stream = fopen(m_PathPairFile.c_str(), "r");          
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                return -1;
        }

        char*  lineBuf = NULL;
        size_t lineLen = 0;
        string line; 
        string key;
        string srcPath; 
        string destPath; 
        map<string, string> mss;

        while (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf;         
                if (0 == line.find('\n') || line.empty() || line.find('[') != string::npos) { 
                        continue; 
                } else {                                                        //字段
                        srcPath  = line.substr(0, line.find(' '));          
                        destPath = line.substr(line.find(' ') + 1);
                        destPath.erase(destPath.find('\n'));
                        pathMap.insert(make_pair(srcPath, destPath));
                }
        }

        if (fclose(stream) != 0) {
                LOG_ERROR("close stream error"); 
        }
        return 0;
}
