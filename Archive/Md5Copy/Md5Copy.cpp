#include "Utils/Log/Log.h"
#include "Config/IniParser.h"
#include "Archive/Md5Copy/Md5Copy.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

Md5Copy::Md5Copy()
{

}

Md5Copy::~Md5Copy()
{
}

int Md5Copy::CopyData(int chkSig, const string& srcPath, const string& destPath)
{
        int ret;
        string cmdLine;

        string tmpSrcPath  = "'" + srcPath  + "'";
        string tmpDestPath = "'" + destPath + "'";
        if (chkSig == 1) {
                cmdLine = "rsync -Rac " + tmpSrcPath + " " + tmpDestPath;
        } else {
                cmdLine = "rsync -Ra "  + tmpSrcPath + " " + tmpDestPath;
        }

        ChildProcessOpr cpo;
        ret = cpo.ExecuteCmd(cmdLine);
        if (ret < 0) {
                LOG_ERROR("ExcuteCmd Error! cmdLine=" << cmdLine);
        }
        return ret;
}

