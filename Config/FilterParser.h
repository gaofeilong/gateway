#ifndef _FILTER_PARSER_
#define _FILTER_PARSER
#include <map>
#include <string>
#include <vector>

class FilterParser {
public:
        explicit FilterParser(const std::string& filterFile = "/etc/scigw/filter.cnf");
        ~FilterParser();
public:
        int Write(const std::map<std::string, std::vector<std::string> >& filter);
        int Read(std::map<std::string, std::vector<std::string> >& filter);
private:
        const std::string m_FilterFile;
};

#endif  //_FILTER_PARSER_
