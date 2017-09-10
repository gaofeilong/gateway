#zhouxy 2010.10.22
#Makefile

proj_dir=./
obj_dir=$(proj_dir)obj/
src_dir=./

#exclude files or dirs by path."" is needed when using a regular expression
exclude_path=

#exclude files or dirs by its name."" is needed when using a regular expression
exclude_name=DDFSArchive.cpp DDFSArvMove.cpp NodeClient.cpp DeleteData.cpp DeleteBackupData.cpp
include_suffix="*.c" "*.cpp"

#the target`s name must be the same as the file with main function.
target=DDFSArchive DDFSArvMove NodeClient DeleteData DeleteBackupData
target_main_src_suffix=.cpp

cpp=ccache g++
cc=gcc
ld=g++
CFLAGS=-c -g -Wall -O2
#CFLAGS=-c -g -Wall -Wno-invalid-offsetof -D_FILE_OFFSET_BITS=64 -D_LOG4CPP -D_LOGFILE -D_ENABLE_IO_MONITOR -D__STDC_LIMIT_MACROS#-D_ENABLE_LOG_DEBUG
#if you need debug info, please add "-D_ENABLE_LOG_DEBUG"
#if you need monitor io, please add "-D_ENABLE_IO_MONITOR"
#if your platform support 64-bit __sync*, please add "-D_GCC_ATOMIC64"

LFLAGS=
#libs=-lpthread -L/usr/local/lib/mysql/ -lmysqlclient
libs=-lpthread -L/usr/local/lib64/scigw -lmysqlclient -lboost_thread -lstxr -laci -Wl,-rpath,/etc/scigw/lib
#libs=-lpthread -I /usr/local/mysql/include/mysql -lmysqlclient
#libs=-lpthread -ltcmalloc_minimal -lboost_thread -llog4cpp -ltokyocabinet -lkyotocabinet -lboost_serialization -lfuse

#usually,you need not change the statements below

###################################################################################
override CFLAGS += -I./

exclude_path_str=$(foreach str,$(exclude_path), -path $(str) -o )

exclude_name_str_=$(foreach str,$(exclude_name), -name $(str) -o )
exclude_name_str=$(wordlist 1,$(shell expr $(words $(exclude_name_str_)) - 1),$(exclude_name_str_))

include_suffix_str_=$(foreach str,$(include_suffix), -name $(str) -o )
include_suffix_str=$(wordlist 1,$(shell expr $(words $(include_suffix_str_)) - 1),$(include_suffix_str_))

dir_src_file=$(shell find ./ \( \( $(exclude_path_str) $(exclude_name_str) \) -prune -o \( $(include_suffix_str) \) -type f -print \) )

src_file=$(foreach obj,$(dir_src_file),$(subst ./,, $(obj)))

obj_file=$(patsubst %, $(obj_dir)%o, $(src_file))

all:$(target)

$(target):%:$(obj_file) $(obj_dir)%$(target_main_src_suffix)o
	$(ld) $^ $(LFLAGS) $(libs) -o $@

$(obj_dir)%o:$(src_dir)%
	@mkdir -p $(obj_dir)$(dir $<)
	$(cpp) $(CFLAGS) $< -o $@

.PHONY:clean
clean:
	rm -f $(target) core*
	rm -fR $(obj_dir)
	rm -f tags

