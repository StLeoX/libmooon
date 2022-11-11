#!/bin/sh
# Write by yijian on 2014/12/27
# 用来生成any2string.h文件
# any2string()是一个类型安全的仿变参函数
# 可用来替代类型不安全的sprintf()和snprintf()等函数
# 带一个参数：最多支持的参数个数值
# 使用方法（20支持的最多参数个数）: any2string.sh 30

# 检查输入参数
if test $# -ne 1; then
	printf "\033[1;33musage: any2string argc\033[m\n"
	exit 1
fi

# .h文件名
h_filename=any2string.h
rm -f $h_filename
# 测试文件名
test_cpp_filename=test_any2string.cpp
rm -f $test_cpp_filename

# 生成单个参数个数为$1指定的函数
generate_single_any2string()
{
	local jj=0
	local argc=$1

	# template <typename T1>
	jj=0
	echo -n "template <" >> $h_filename
	while test $jj -lt $argc; do
		if test $jj -eq 0; then
			echo -n "typename T$jj" >> $h_filename
		else
			echo -n ", typename T$jj" >> $h_filename
		fi

		jj=$(($jj + 1))
	done	
	echo ">" >> $h_filename
	
	# std::string any2string(T1 t1)
	jj=0	
	echo -n "inline std::string any2string(" >> $h_filename
	while test $jj -lt $argc; do
		if test $jj -eq 0; then
			echo -n "const T$jj& t$jj" >> $h_filename
		else
			echo -n ", const T$jj& t$jj" >> $h_filename
		fi

		jj=$(($jj + 1))
	done	
	echo ")" >> $h_filename
	
	# 函数体
	jj=0
	echo "{" >> $h_filename
	echo "    std::stringstream ss;" >> $h_filename
	echo -n "    ss" >> $h_filename
	while test $jj -lt $argc; do
		echo -n " << t$jj" >> $h_filename
		jj=$(($jj + 1))
	done
	echo ";" >> $h_filename
	
	# 返回值
	echo "    return ss.str();" >> $h_filename
	echo "}" >> $h_filename
}

# 生成所有的函数
generate_all_any2string()
{
	local ii=1
	local argc=$1

	while test $ii -le $argc; do
		generate_single_any2string $ii
		ii=$(($ii + 1))

		echo "" >> $h_filename
	done
}

# 生成any2string.h文件函数
generate_any2string_h()
{
	local argc=$1

	# 输出文件头
	echo "// Write by yijian on 2014/12/27" >> $h_filename
	echo "// DO NOT EDIT!" >> $h_filename
	echo "// this header file is auto generated by any2string.sh" >> $h_filename
	echo "// edit any2string.sh if necessary" >> $h_filename
	echo "//" >> $h_filename
	echo "// 类型安全的变参函数，可用来替代类型不安全的sprintf()和snprintf()等函数" >> $h_filename
	echo "// 使用示例1: std::string str = any2string(20141227);" >> $h_filename
	echo "// 使用示例2: std::string str = any2string(20141227, \"22:07:10\");" >> $h_filename
	echo "// 使用示例3: std::string str = any2string(1, \"2\", '3', std::string(\"4\"));" >> $h_filename

	echo "#ifndef MOOON_UTIL_ANY2STRING_H" >> $h_filename
	echo "#define MOOON_UTIL_ANY2STRING_H" >> $h_filename
	echo "#include \"mooon/utils/config.h\"" >> $h_filename
	echo "#include <string>" >> $h_filename
	echo "#include <sstream>" >> $h_filename
	echo "UTIL_NAMESPACE_BEGIN" >> $h_filename
	echo "" >> $h_filename
	
	# 输出文件体
	generate_all_any2string $argc

	# 输出文件尾
	echo "" >> $h_filename
	echo "UTIL_NAMESPACE_END" >> $h_filename
	echo "#endif // MOOON_UTIL_ANY2STRING_H" >> $h_filename
	echo "" >> $h_filename
}

# 生成用于测试any2string.h的测试文件test_any2string.cpp
generate_test_any2string()
{
	local argc=$1

	echo "// Write by yijian on 2014/12/27" >> $test_cpp_filename
	echo "// used to test any2string.h" >> $test_cpp_filename
	echo "#include \"any2string.h\"" >> $test_cpp_filename
	echo "#include <stdio.h>" >> $test_cpp_filename
	echo "" >> $test_cpp_filename
	
	echo "int main()" >> $test_cpp_filename
	echo "{" >> $test_cpp_filename
	
	if test $argc -gt 0; then
		echo "    printf(\"%s\\n\", any2string(1).c_str());" >> $test_cpp_filename
	fi
	if test $argc -gt 1; then
		echo "    printf(\"%s\\n\", any2string(1, \"2\").c_str());" >> $test_cpp_filename
	fi
	if test $argc -gt 2; then
		echo "    printf(\"%s\\n\", any2string(1, \"2\", 3).c_str());" >> $test_cpp_filename
	fi
	if test $argc -gt 3; then
		echo "    printf(\"%s\\n\", any2string(1, \"2\", 3, \"4\").c_str());" >> $test_cpp_filename
	fi
	if test $argc -gt 4; then
		echo "    printf(\"%s\\n\", any2string(1, '-', \"2\", \"-\", 3, \"-\", \"4\", '-', \"5\").c_str());" >> $test_cpp_filename
	fi
	if test $argc -gt 9; then
		echo "    printf(\"%s\\n\", any2string(\"https\", ':', \"//\", \"github\", '.', std::string(\"com\"), '/', \"eyjian\", '/', std::string(\"mooon\")).c_str());" >> $test_cpp_filename
	fi

	echo "" >> $test_cpp_filename
	echo "    return 0;" >> $test_cpp_filename
	echo "}" >> $test_cpp_filename
	echo "" >> $test_cpp_filename
}

# 生成any2string.h文件
generate_any2string_h $1
generate_test_any2string $1

exit 0
