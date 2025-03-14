# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
# 定义公共的、可直接复用的
# 注意使用静态库时有顺序要求，假设静态库libx.a依赖于libz.a，则指定顺序须为-lx -lz（或libx.a libz.a），不能反过来为-lz -lx（或libz.a libx.a）。
#
# 约定：
# 安装第三方库时，明确指定prefix，即指定安装目录方式，不管是automake还是CMake等方式
#
# 搜索路径按优先顺序分为三种：
# 1）由xxx_HOME环境变量指定的路径，比如：export MYSQL_HOME=/usr/local/mysql
# 2）标准路径/usr/local/xxx，比如：/usr/local/mysql或/usr/local/hiredis等
# 3）准标准路径/usr/local/thirdparty，比如：/usr/local/thirdparty/mysql
# 4）默认路径/usr/local，如：/usr/local/include/mysql
# 其它路径将不能被自动识别和发现。

# 让make时显示编译命令
set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_C_COMPILER /usr/local/gcc/bin/gcc)
#set(CMAKE_CXX_COMPILER /usr/local/gcc/bin/c++)

# 定义颜色值，message()时可用到
if (NOT WIN32)
    string(ASCII 27 Esc)
    set(ColourReset "${Esc}[0m")
    set(ColourBold  "${Esc}[1m")
    set(Red         "${Esc}[31m")
    set(Green       "${Esc}[32m")
    set(Yellow      "${Esc}[33m")
    set(Blue        "${Esc}[34m")
    set(Magenta     "${Esc}[35m")
    set(Cyan        "${Esc}[36m")
    set(White       "${Esc}[37m")
    set(BoldRed     "${Esc}[1;31m")
    set(BoldGreen   "${Esc}[1;32m")
    set(BoldYellow  "${Esc}[1;33m")
    set(BoldBlue    "${Esc}[1;34m")
    set(BoldMagenta "${Esc}[1;35m")
    set(BoldCyan    "${Esc}[1;36m")
    set(BoldWhite   "${Esc}[1;37m")
endif ()

# 检查是32位还是64位系统
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    message("${Yellow}x86 system${ColourReset}")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message("${Yellow}x86_64 system${ColourReset}")
else ()
    message("${Yellow}unknown bits system${ColourReset}")
endif ()

#
# 只要MySQL、Boost、Thrift等安装在下述三个目录，即可自动发现：
# 1) /usr/local/thirdparty
# 2) /usr/local
# 3) 用户主目录，即环境变量$HOME对应的目录
#

# find_library/find_path/find_package/find_program
# find_package要求目标目录有.cmake文件
#
# find_path使用示例：
# find_path(
#     HIREDIS_INCLUDE
#     NAMES
#     hiredis.h
#     PATHS 
#     /usr/local/hiredis/include/hiredis
#     /usr/include
#     NO_DEFAULT_PATH
# )
# HIREDIS_INCLUDE的值为HIREDIS_INCLUDE-NOTFOUND（如果未找到）或/usr/local/hiredis/include/hiredis或/usr/include（如果找到）
# include_directories(${HIREDIS_INCLUDE}/..)
# if ("${HIREDIS_INCLUDE}" STREQUAL "HIREDIS_INCLUDE-NOTFOUND"
#
# 自动寻找库的安装位置函数
# discover_library 函数名
# uppername 库的大写名称，如MYSQL
# dirname 库的目录名，如mysql
#
# 也可以通过cmake参数指定，比如：“cmake -DMYSQL_HOME=/home/mike/mysql .”这种形式
function(discover_library uppername dirname)
    if (${uppername}_HOME)
    	if (EXISTS ${${uppername}_HOME})
        	set(MOOON_HAVE_${uppername} 1 CACHE INTERNAL MOOON_HAVE_${uppername})
       	else ()
       		set(MOOON_HAVE_${uppername} 0 CACHE INTERNAL MOOON_HAVE_${uppername})
       	endif ()
    elseif (EXISTS /usr/local/${dirname})
        set(MOOON_HAVE_${uppername} 1 CACHE INTERNAL MOOON_HAVE_${uppername})
        set(${uppername}_HOME /usr/local/${dirname} CACHE INTERNAL ${uppername}_HOME)
    elseif (EXISTS /usr/local/thirdparty/${dirname})
        # 如果使用PARENT_SCOPE，则函数内不能识别，但函数外可以
        #set(MOOON_HAVE_${uppername} 1 PARENT_SCOPE)
        set(MOOON_HAVE_${uppername} 1 CACHE INTERNAL MOOON_HAVE_${uppername})
        set(${uppername}_HOME /usr/local/thirdparty/${dirname} CACHE INTERNAL ${uppername}_HOME)
    elseif (EXISTS $ENV{HOME}/${dirname})
        #set(MOOON_HAVE_${uppername} 1 PARENT_SCOPE CACHE INTERNAL MOOON_HAVE_${uppername})
        set(MOOON_HAVE_${uppername} 1 CACHE INTERNAL MOOON_HAVE_${uppername})
        set(${uppername}_HOME $ENV{HOME}/${dirname} CACHE INTERNAL ${uppername}_HOME)
    elseif (EXISTS /usr/local/include/${dirname})
        set(MOOON_HAVE_${uppername} 1 CACHE INTERNAL MOOON_HAVE_${uppername})
        set(${uppername}_HOME /usr/local CACHE INTERNAL ${uppername}_HOME)
    else ()
        set(MOOON_HAVE_${uppername} 0 CACHE INTERNAL MOOON_HAVE_${uppername})
    endif ()

    if (NOT ${MOOON_HAVE_${uppername}})
    	if (${uppername}_HOME)
        	message("${Green}not found ${uppername} in ${uppername}_HOME${ColourReset}")
       	else ()
       		message("${Green}not found ${uppername}${ColourReset}")
       	endif ()
    else ()
        message("${Red}${uppername} found in ${${uppername}_HOME}${ColourReset}")
        add_definitions("-DMOOON_HAVE_${uppername}=1")
        include_directories(${${uppername}_HOME}/include)
        link_directories(${${uppername}_HOME}/lib)
        #link_libraries(libmysqlclient_r.a)
    endif ()
endfunction ()

# MySQL
discover_library(MYSQL mysql)
#link_libraries(libmysqlclient_r.a) # 新版本已经没有libmysqlclient_r.a，只有libmysqlclient.a
#link_libraries(libmysqlclient.a)
if (MOOON_HAVE_MYSQL)    
    if (EXISTS ${MYSQL_HOME}/lib/mysql/libmysqlclient_r.a)
        message("using libmysqlclient_r.a")
        link_directories(${MYSQL_HOME}/lib/mysql)
        set(MOOON_MYSQLLIB libmysqlclient_r.a)
    elseif (EXISTS ${MYSQL_HOME}/lib/mysql/libmysqlclient.a)
        message("using libmysqlclient.a")
        link_directories(${MYSQL_HOME}/lib/mysql)
        set(MOOON_MYSQLLIB libmysqlclient.a)
    elseif (EXISTS ${MYSQL_HOME}/lib/libmysqlclient_r.a)
        message("using libmysqlclient_r.a")
        link_directories(${MYSQL_HOME}/lib)
        set(MOOON_MYSQLLIB libmysqlclient_r.a)
    elseif (EXISTS ${MYSQL_HOME}/lib/libmysqlclient.a)
        message("using libmysqlclient.a")
        link_directories(${MYSQL_HOME}/lib)
        set(MOOON_MYSQLLIB libmysqlclient.a)
    endif ()
endif ()

# 可控制是否使用SQLite3，默认是不使用的
# SQLite3
discover_library(SQLITE3 sqlite3)
#link_libraries(libsqlite3.a)
# SQLite3安装后的include目录结构不标准，调整成标准结构
if (MOOON_HAVE_SQLITE3)
    if (NOT EXISTS ${SQLITE3_HOME}/include/sqlite3)
        message("${Red}SQLITE3 directory nonstandard${ColourReset}")
        exec_program(mkdir ARGS ${SQLITE3_HOME}/include/sqlite3)
        exec_program(cp ARGS ${SQLITE3_HOME}/include/sqlite3.h ${SQLITE3_HOME}/include/sqlite3/)
        exec_program(cp ARGS ${SQLITE3_HOME}/include/sqlite3ext.h ${SQLITE3_HOME}/include/sqlite3/)
    endif ()
endif ()

# idn（一些环境Curl强依赖idn）
discover_library(LIBIDN libidn)
#link_libraries(libidn.a)

# C-ares（Curl可选依赖C-ares）
discover_library(CARES c-ares)
#link_libraries(libcares.a)

# Curl（Curl可选依赖C-ares、OpenSSL等）
discover_library(CURL curl)
#link_libraries(libcurl.a)

# Cgicc
discover_library(CGICC cgicc)
#link_libraries(libcgicc.a)

# libunwind
discover_library(LIBUNWIND libunwind)
#link_libraries(libunwind.a libunwind-generic.a libunwind-coredump.a libunwind-ptrace.a libunwind-setjmp.a)
  
# gperftools
discover_library(GPERFTOOLS gperftools)
#link_libraries(libtcmalloc.a libtcmalloc_debug.a libtcmalloc_minimal.a libtcmalloc_minimal_debug.a libtcmalloc_and_profiler.a libprofiler.a)

# Sparsehash
discover_library(SPARSE_HASH sparsehash)
# Sparsehash只有头文件

# Libevent
discover_library(LIBEVENT libevent)
#link_libraries(libevent.a)

# libssh2
# http://www.libssh2.org/
# libssh
# http://www.libssh.org/
discover_library(LIBSSH2 libssh2)
#link_libraries(libssh2.a)

# Boost
discover_library(BOOST boost)
#link_libraries(libboost_filesystem.a libboost_thread.a libboost_system.a libboost_date_time.a)

# Thrift
discover_library(THRIFT thrift)
#link_libraries(libthriftnb.a libthrift.a)

# jsoncpp
discover_library(JSONCPP jsoncpp)
#link_libraries(libjsoncpp.a)

# rapidjson
discover_library(RAPIDJSON rapidjson)
# rapidjson纯头文件，没有库文件

# rapidxml
discover_library(RAPIDXML rapidxml)
# rapidxml纯头文件，没有库文件

# OpenSSL
discover_library(OPENSSL openssl)
#link_libraries(libssl.a libcrypto.a)

# Protocol Buffers
discover_library(PROTOBUF protobuf)
#link_libraries(libprotobuf.a)

# zookeeper
discover_library(ZOOKEEPER zookeeper)
#link_libraries(libzookeeper_st.a)
#link_libraries(libzookeeper_mt.a)

# hiredis
discover_library(HIREDIS hiredis)
#link_libraries(libhiredis.a)

# r3c (based on hiredis)
discover_library(R3C r3c)
#link_libraries(libr3c.a)

# leveldb
discover_library(LEVELDB leveldb)
#link_libraries(libleveldb.a libmemenv.a)

# librdkafka
discover_library(LIBRDKAFKA librdkafka)
#link_libraries(librdkafka++.a librdkafka.a)

# 编译参数
# 启用__STDC_FORMAT_MACROS是为了可以使用inttypes.h中的PRId64等
# 启用__STDC_LIMIT_MACROS是为了可以使用stdint.h中的__UINT64_C和INT32_MIN等
add_definitions("-Wall -Wno-volatile -Wno-unused -Wno-format-truncation -fPIC -pthread -D_GNU_SOURCE -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS")

# 代码中如有使用到atomic，则和-march有关
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    message("${Yellow}pentium4${ColourReset}")
    add_definitions("-march=pentium4")    
endif ()

include(CheckCXXCompilerFlag)
# 如果编译环境支持C++20，则打开C++20开关
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
if (COMPILER_SUPPORTS_CXX20)
    message("${Yellow}enable C++20${ColourReset}")
    add_definitions("-std=c++20")
else ()
    message("${Yellow}not support C++20${ColourReset}")

    # 如果编译环境支持C++17，则打开C++17开关
    CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
    if (COMPILER_SUPPORTS_CXX17)
        message("${Yellow}enable C++17${ColourReset}")
        add_definitions("-std=c++17")
    else ()
        message("${Yellow}not support C++17${ColourReset}")

        # 如果编译环境支持C++14，则打开C++14开关
        CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
        if (COMPILER_SUPPORTS_CXX14)
            message("${Yellow}enable C++14${ColourReset}")
            add_definitions("-std=c++14")
        else ()
            message("${Yellow}not support C++14${ColourReset}")
            
            # 如果编译环境支持C++11，则打开C++11开关
            CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
            if (COMPILER_SUPPORTS_CXX11)
                message("${Yellow}enable C++11${ColourReset}")
                #add_definitions("-std=c++11")
                add_compile_options("-std=c++11")
            else ()
                message("${Yellow}not support C++11${ColourReset}")
            endif()
        endif()
    endif()
endif()

# 公共的库
# 如果链接libdl.a或libpthread.a或librt.a或libz.a，就不适合作为公共操作，
# 如：-Wl,-Bstatic -static-libgcc -lrt -lz -pthread -ldl
#link_libraries(-pthread dl rt z) # 注意这里如使用“pthread”，则实际可能变成“-lpthread”

# 为指定的源文件添加编译属性，示例：
# set_source_files_properties(example1.cpp example2.cpp COMPILE_FLAGS -DXXXX=1234)
# set_source_files_properties(example1.cpp example2.cpp PROPERTIES COMPILE_FLAGS -DXXXX=1234)

# 为指定的目标设置属性，示例：
# set_target_properties(target1 target2 PROPERTIES LINK_FLAGS -lrt)
# 示例2：
# set_target_properties(test PROPERTIES COMPILE_FLAGS -DXXXX=1234)
#
# 设置前缀后缀，如果没有下面这一句，则生成的文件名为libtest.fcgi.so
# set_target_properties(test.fcgi PROPERTIES PREFIX "" SUFFIX "")
# 设置目录属性
# set_directory_properties
# 在给定的作用域内设置一个命名的属性  
# set_property

# 设置依赖
# ADD_DEPENDENCIES(Target 被依赖的Target)
