#ifndef _CRONT_
#define _CRONT_

#include <string>
using std::string;

class Cront {
public:
        Cront();
        ~Cront();
public:
        int Find(const string& cmd);
        int AddCront(const string& configPath);
        int DelCront(const string& configPath);
};

#endif //_CRONT_
