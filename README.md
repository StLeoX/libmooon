## 简介

libmooon是一个C++静态工具类库，特性在于兼容C++11之前的环境。  
如果是C++11或以上开发环境，libmooon中的一些设施可直接由C++标准库提供。

> 如std::thread替代mooon::sys::CThreadEngine，std::mutex替代mooon::sys::CLock。  
> **建议尽量使用标准库提供的基础设施**，libmooon诞生于C++11之前，可应用于C++11之前的开发环境。

## 安装

使用CMake编译和安装libmooon分三个步骤：

1. 执行cmake命令生成Makefile文件，命令格式：cmake -DCMAKE_INSTALL_PREFIX=<installation directory> ；

2. 执行make命令编译libmooon源代码；

3. 执行make install安装头文件和库文件等；

libmooon所依赖的第三方库是可选的，即使第三方库不存在，也能正常编译libmooon，但会缺乏相应的功能模块。

> 比如在执行cmake命令时，未能检测到MySQL，则src/sys/mysql_db.cpp实际为空，同时对应的宏MOOON_HAVE_MYSQL值为0，
> 也支持cmake命令行参数指定第三库的安装位置，如：cmake DMYSQL_HOME=/home/mike/mysql

指定安装目录：“-DCMAKE_INSTALL_PREFIX=”后跟安装目录。

> 示例：指定安装目录为 /usr/local/mooon
> 选项：cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local/mooon .

指定编译方式。

> 如果以debug方式编译，指定参数：-DCMAKE_BUILD_TYPE=Debug
> 如果以Release方式编译，指定参数：-DCMAKE_BUILD_TYPE=Release

## 使用建议

注意：使用静态库时有顺序要求，假设静态库libx.a依赖于libz.a， 则指定顺序须为-lx -lz（或libx.a libz.a），不能反过来为-lz -lx（或libz.a libx.a）， 当然也可以使用链接参数“-Wl,--startgroup”和“-Wl,--endgroup”忽略静态库的顺序。

当前libmooon.a直接依赖的可选第三方库列表：

1. curl（src/sys/curl_wrapper.cpp，curl又依赖openssl、c-ares和libidn）
2. mysql（src/sys/mysql_db.cpp）
3. sqlite3（src/sys/sqlite3_db.cpp）
4. zookeeper（include/net/zookeeper_helper.h）
5. thrift（include/net/thrift_helper.h，thrift又依赖openssl、libevent和boost）
6. openssl（src/utils/aes_helper.cpp，thrift也依赖openssl）
7. libssh2（sys/net/libssh2.cpp，libssh2又依赖openssl）

可将所有直接和间接依赖的第三方库安装到“/usr/local”目录下，这样在执行cmake命令行时，将自动发现它们，例如mysql的安装目录为“/usr/local/mysql”。

以下第三方库，libmooon库本身未直接和间接依赖，因此不是必须的：

1. hiredis
2. r3c
3. leveldb
4. protobuf
5. rapidxml
6. rapidjson
7. jsoncpp
8. Sparsehash
9. gperftools
10. libunwind

## FAQs


