#include <stdio.h>
#include "Config/FilterParser.h"
#include "Utils/Log/Log.h"
#include "Utils/CommonOpr/DirFileOpr.h"

using std::map;
using std::string;
using std::vector;

FilterParser::FilterParser(const string& filterFile):m_FilterFile(filterFile)
{
}

FilterParser::~FilterParser()
{
}

int FilterParser::Write(const map<string, vector<string> >& filter)
{
        FILE* stream = fopen(m_FilterFile.c_str(), "w");          
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                return -1;
        }

        map<string, vector<string> >::const_iterator msvIter = filter.begin();
        for ( ; msvIter != filter.end(); ++msvIter) {
                fprintf(stream, "[%s]\n", msvIter->first.c_str());
                vector<string>::const_iterator vsIter = msvIter->second.begin();
                for ( ; vsIter != msvIter->second.end(); ++vsIter) {
                        fprintf(stream, "%s\n", vsIter->c_str());
                }
        }

        if (fclose(stream) != 0) {
                LOG_ERROR("close stream error"); 
        }
        return 0;
}

int FilterParser::Read(map<string, vector<string> >& filter)
{
        DirFileOpr dfo;
        if (!dfo.HasPath(m_FilterFile)) {
                return 0;
        }

        FILE* stream = fopen(m_FilterFile.c_str(), "r");          
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                return -1;
        }

        char*          lineBuf = NULL;
        size_t         lineLen = 0;
        string         line; 
        string         key;
        vector<string> vs;

        while (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf;         
                if (0 == line.find('\n') || line.empty()) {                     //空行
                        continue; 
                } else if (line.find('[') != string::npos) {                    //域名
                        if (!key.empty() && !vs.empty()) {                      //
                                filter.insert(make_pair(key, vs));                                
                                key.clear();
                                vs.clear();
                        }
                        line.erase(line.find('['), 1); 
                        line.erase(line.find(']'));
                        key = line;
                } else {
                        line.erase(line.find('\n')); 
                        vs.push_back(line);
                }
        }
        filter.insert(make_pair(key, vs));

        if (fclose(stream) != 0) {
                LOG_ERROR("close stream error"); 
        }
        return 0;
}
