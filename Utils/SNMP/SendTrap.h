#if !defined(_SENDTRAP_H)
#define _SENDTRAP_H

#include <string>
using std::string;

class SendTrap
{
        public:
                SendTrap();
                ~SendTrap();
        public:
                int Send(const string& ip, const string& oid, const string& info);
};
#endif

