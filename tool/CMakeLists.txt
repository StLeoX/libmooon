# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(.)
include_directories(../include)
include_directories(../include/mooon)
link_directories(../src)
link_libraries(dl pthread rt z)

# 计算文件信息摘要
if (MOOON_HAVE_OPENSSL)
    add_executable(mooon_md5 md5.cpp)
    target_link_libraries(mooon_md5 libmooon.a libcrypto.a)

    add_executable(mooon_base64 base64.cpp)
    target_link_libraries(mooon_base64 libmooon.a libcrypto.a)

    add_executable(mooon_sha sha.cpp)
    target_link_libraries(mooon_sha libmooon.a libcrypto.a)
endif ()

# 测试硬盘性能
add_executable(mooon_disk_benchmark disk_benchmark.cpp)
target_link_libraries(mooon_disk_benchmark libmooon.a)
# CMAKE_INSTALL_PREFIX
install(
    TARGETS mooon_disk_benchmark
    DESTINATION bin
)

# 根据进程名获取进程号
add_executable(mooon_pidof pidof.cpp)
target_link_libraries(mooon_pidof libmooon.a)

# 根据进程名杀死全部进程
add_executable(mooon_killall killall.cpp)
target_link_libraries(mooon_killall libmooon.a)

if (MOOON_HAVE_LIBSSH2)
	# SSH多连接，执行命令
	add_executable(mooon_ssh mooon_ssh.cpp)
	target_link_libraries(mooon_ssh libmooon.a libssh2.a libcrypto.a)

	# SSH多连接，上传文件
	add_executable(mooon_upload mooon_upload.cpp)
	target_link_libraries(mooon_upload libmooon.a libssh2.a libcrypto.a)

    # SSH多连接，下载文件
    add_executable(mooon_download mooon_download.cpp)
    target_link_libraries(mooon_download libmooon.a libssh2.a libcrypto.a)

	# CMAKE_INSTALL_PREFIX
	install(
	        TARGETS mooon_ssh mooon_upload mooon_download
	        DESTINATION bin
	)
endif ()

# r3c_stress
if (MOOON_HAVE_R3C)
    # r3c_stress
    #add_executable(r3c_stress r3c_stress.cpp)
    #target_link_libraries(r3c_stress libmooon.a libr3c.a libhiredis.a)

    # redis_queue_mover
    add_executable(redis_queue_mover redis_queue_mover.cpp)
    target_link_libraries(redis_queue_mover libmooon.a libr3c.a libhiredis.a)

	# queue_redis2redis
	add_executable(queue_redis2redis queue_redis2redis.cpp)
	target_link_libraries(queue_redis2redis libmooon.a libr3c.a libhiredis.a libssl.a libcrypto.a)
	
	# queue_file2redis
	add_executable(queue_file2redis queue_file2redis.cpp)
	target_link_libraries(queue_file2redis libmooon.a libr3c.a libhiredis.a libssl.a libcrypto.a)
	
	# queue_redis2file
	add_executable(queue_redis2file queue_redis2file.cpp)
	target_link_libraries(queue_redis2file libmooon.a libr3c.a libhiredis.a libssl.a libcrypto.a)
	
    # CMAKE_INSTALL_PREFIX
    install(
            TARGETS redis_queue_mover queue_redis2redis queue_file2redis queue_redis2file
            DESTINATION bin
	)
endif ()

# hbase_stress
if (MOOON_HAVE_THRIFT)
    exec_program(${THRIFT_HOME}/bin/thrift ARGS --gen cpp -r -out ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/hbase.thrift)
    exec_program(rm ARGS ${CMAKE_CURRENT_SOURCE_DIR}/THBaseService_server.skeleton.cpp)

    add_executable(hbase_stress hbase_stress.cpp THBaseService.cpp hbase_types.cpp)
    target_link_libraries(hbase_stress libmooon.a libthrift.a)

    add_executable(hbase_scan hbase_scan.cpp THBaseService.cpp hbase_types.cpp)
    target_link_libraries(hbase_scan libmooon.a libthrift.a)

    # CMAKE_INSTALL_PREFIX
    install(
            TARGETS hbase_stress hbase_scan
            DESTINATION bin
	)
endif ()

# thrift_server & thrift_client
#if (MOOON_HAVE_THRIFT)
#    exec_program(${THRIFT_HOME}/bin/thrift ARGS --gen cpp -r -out ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/test.thrift)
#    exec_program(rm ARGS ${CMAKE_CURRENT_SOURCE_DIR}/TestService_server.skeleton.cpp)
#
#    add_executable(thrift_server thrift_server.cpp TestService.cpp)
#    target_link_libraries(thrift_server libmooon.a libthriftnb.a libthrift.a libevent.a)
#
#    add_executable(thrift_client thrift_client.cpp TestService.cpp)
#    target_link_libraries(thrift_client libmooon.a libthriftnb.a libthrift.a libevent.a)
#
#    # CMAKE_INSTALL_PREFIX
#    install(
#            TARGETS thrift_server thrift_client
#            DESTINATION bin
#	)
#endif ()

# mysql_escape_test
if (MOOON_HAVE_MYSQL)
    add_executable(mysql_escape_test mysql_escape_test.cpp)
    target_link_libraries(mysql_escape_test libmooon.a libmysqlclient.a)
    install( # CMAKE_INSTALL_PREFIX
        TARGETS mysql_escape_test
        DESTINATION bin
    )

    add_executable(mysql_table_copy mysql_table_copy.cpp)
    target_link_libraries(mysql_table_copy libmooon.a libmysqlclient.a)
    install( # CMAKE_INSTALL_PREFIX
        TARGETS mysql_table_copy
        DESTINATION bin
    )
endif ()

if (MOOON_HAVE_ZOOKEEPER)
    # zkupload
    add_executable(zkupload zk_upload.cpp)
    target_link_libraries(zkupload libmooon.a libzookeeper_mt.a)

    # zkdownload
    add_executable(zkdownload zk_download.cpp)
    target_link_libraries(zkdownload libmooon.a libzookeeper_mt.a)

    # CMAKE_INSTALL_PREFIX
    install(
            TARGETS zkupload zkdownload
            DESTINATION bin
	)
endif ()

if (MOOON_HAVE_CURL)
    # curl_download
    add_executable(curl_download curl_download.cpp)
    target_link_libraries(curl_download libmooon.a libcurl.a libcares.a libidn.a libssl.a libcrypto.a)
endif ()

if (MOOON_HAVE_LIBRDKAFKA)
	add_executable(kafka_consumer kafka_consumer.cpp)
    target_link_libraries(kafka_consumer libmooon.a librdkafka++.a librdkafka.a libssl.a libcrypto.a)

	# CMAKE_INSTALL_PREFIX
    install(
            TARGETS kafka_consumer
            DESTINATION bin
	)
endif ()

if (MOOON_HAVE_R3C AND MOOON_HAVE_LIBRDKAFKA)
	# queue_redis2kafka
	add_executable(queue_redis2kafka queue_redis2kafka.cpp)
	target_link_libraries(queue_redis2kafka libmooon.a libr3c.a libhiredis.a librdkafka++.a librdkafka.a libssl.a libcrypto.a)

	# queue_kafka2redis
	add_executable(queue_kafka2redis queue_kafka2redis.cpp)
	target_link_libraries(queue_kafka2redis libmooon.a libr3c.a libhiredis.a librdkafka++.a librdkafka.a libssl.a libcrypto.a)
	
	# CMAKE_INSTALL_PREFIX
	install(
		TARGETS queue_redis2kafka queue_kafka2redis
		DESTINATION bin
	)
endif ()
