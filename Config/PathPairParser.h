#ifndef _PATH_PAIR_PARSER_
#define _PATH_PAIR_PARSER
#include <map>
#include <string>
#include <vector>

class PathPairParser {
public:
        explicit PathPairParser(const std::string& filterFile = "/etc/scigw/archive.cnf");
        ~PathPairParser();
public:
        int Write(const std::map<std::string, std::string>& pathMap);
        int Read(std::map<std::string, std::string>& pathMap);
private:
        const std::string m_PathPairFile;
};

#endif  //_PATH_PAIR_PARSER_
