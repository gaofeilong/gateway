#include "Utils/CommonOpr/ChildProcessOpr.h"
#include "Utils/Log/Log.h"
#include "Utils/SNMP/SendTrap.h"

SendTrap::SendTrap()
{

}

SendTrap::~SendTrap()
{

}

int SendTrap::Send(const string& ip, const string& oid, const string& info)
{
        string command;
        string stream = "scidata: " + info;
        command = "snmptrap -v 2c -c public " + ip +":162" + " \"\" " + oid 
                        + " " + oid + ".0 s \"" + stream + "\"";

        ChildProcessOpr cmdOpr;
        if(cmdOpr.ExecuteCmd(command) != 0) {
                LOG_ERROR("sendtrap failed");
                return -1;       
        }
        return 0;
}
