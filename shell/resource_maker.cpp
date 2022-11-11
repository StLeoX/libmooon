// Author: yijian
// Date: 2015/01/23
// Reference: /usr/bin/xxd -i filepath
// C++资源编译工具，用于将任何格式的文件编译成C++代码
// 优点：单个.cpp文件，无其它依赖，一句编译后即可使用
// 编译：g++ -Wall -g -o resource_maker resource_maker.cpp
//
// 编译后，会生成与资源文件对应的.cpp文件，访.cpp文件包含两个全局变量：
// 1) size变量：存储资源文件的字节数大小，变量名同文件名，但不包含扩展名部分
// 2) 资源文件的内容变量：以十六进制方式表达
// 注意，所有变量总是位于resource名字空间内。
//
// 示例，假设就以resource_maker.cpp为资源文件，则：
// 1) 将resource_maker.cpp编译成C++代码：./resource_maker ./resource_maker.cpp
// 2) 可以看到生成了对应的c++代码文件：res_resource_maker.cpp
// 3) 打开res_resource_maker.cpp文件，可以看到的两个resource名字空间内的全局变量：
// size_t resource_maker_size和unsigned char resource_maker[];
//
// 接下来，就可以根据需求使用以变量的形式在c++代码中以只读的方式访问资源文件了，如：
// namespace resource {
//     extern size_t resource_maker_size;
//     extern unsigned char resource_maker[];
// }
// int main()
// {
//     // 因为resource_maker.cpp是文本格式，所以可以printf，
//     // 但如果是图片、音频等二进制格式的文件，显示就不能这样了。
//     printf("%s\n", static_cast<char*>(resource::resource_maker));
//     return 0;
// }
#include <error.h>
#include <fstream>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// 从文件路径中扣出不带斜杠结尾的目录路径
static std::string extract_dirpath_without_slash(const std::string& filepath);

// 从文件路径中扣出不带后缀的文件名
static std::string extract_filename_without_suffix(const std::string& filepath);

// 将一个文件内容全读取出来，放到buffer中
static bool file2string(const std::string& filepath, std::string* buffer);

// 写.h文件
static bool write_h_file(const std::string& resource_h_filepath, const std::string& c_variable_name);

// 写.cpp文件
static bool write_cpp_file(const std::string& resource_cpp_filepath, const std::string& c_variable_name, const std::string& buffer);

// 将一个十进制值转换成十六进制，并带前缀0x，如果不足位字符宽度，则被0
// 如：a变成0x61，1变成0x31，。。。
static std::string dec2hex(unsigned char c);

// 用法，带2个参数：
// 参数1：resource_maker 资源文件
// 参数2：文件名前缀（可选，默认为res_，如果文件名是骆驼命名风格，建议改为Res）
int main(int argc, char* argv[])
{
    std::string filename_prefix = (3 == argc)? argv[2]: "res_";
    std::string resource_filepath = argv[1];
    std::string resource_dirpath = extract_dirpath_without_slash(resource_filepath);
    std::string filename_without_suffix = extract_filename_without_suffix(resource_filepath);

    std::string resource_h_filepath = resource_dirpath + "/" + filename_prefix + filename_without_suffix + ".h";
    std::string resource_cpp_filepath = resource_dirpath + "/" + filename_prefix + filename_without_suffix + ".cpp";
    std::string buffer; // 用来存储资源文件的内容
    std::string c_variable_name = filename_without_suffix; // 用这个变量来存储编码后的资源文件内容

    fprintf(stdout, "h file: %s\n", resource_h_filepath.c_str());
    fprintf(stdout, "cpp file: %s\n", resource_cpp_filepath.c_str());
    fprintf(stdout, "variable name: %s\n", c_variable_name.c_str());

    // 输入参数检查，
    // 要求带一个参数
    if ((argc != 2) && (argc != 3))
    {
        fprintf(stderr, "usage: %s resouce_filepath <filename_prefix>\n", basename(argv[0]));
        exit(1);
    }

    if (!file2string(resource_filepath, &buffer))
    {
        exit(1);
    }

    if (!write_h_file(resource_h_filepath, c_variable_name))
    {
        exit(1);
    }

    if (!write_cpp_file(resource_cpp_filepath, c_variable_name, buffer))
    {
        exit(1);
    }

    return 0;
}

std::string extract_dirpath_without_slash(const std::string& filepath)
{
    char* tmp = strdup(filepath.c_str());
    std::string dirpath_without_slash = dirname(tmp);

    free(tmp);
    return dirpath_without_slash;
}

std::string extract_filename_without_suffix(const std::string& filepath)
{
    char* tmp = strdup(filepath.c_str());

    // 去掉目录部分
    std::string filename = basename(tmp);
    std::string::size_type dot_pos = filename.rfind('.');

    free(tmp);
    return (std::string::npos == dot_pos)? filename: filename.substr(0, dot_pos);
}

bool file2string(const std::string& filepath, std::string* buffer)
{
    std::ifstream fs(filepath.c_str());
    if (!fs)
    {
        fprintf(stderr, "open %s error: %m\n", filepath.c_str());
        return false;
    }

    // 得到文件大小
    fs.seekg(0, std::ifstream::end);
    std::streamoff file_size = fs.tellg();

    // 调整buffer大小
    buffer->resize(static_cast<std::string::size_type>(file_size + 1));
    (*buffer)[file_size] = '\0';

    // 将整个文件读到buffer中
    fs.seekg(0, std::ifstream::beg);
    fs.read(const_cast<char*>(buffer->data()), file_size);

    return true;
}

bool write_h_file(const std::string& resource_h_filepath, const std::string& c_variable_name)
{
    return true;
}

bool write_cpp_file(const std::string& resource_cpp_filepath, const std::string& c_variable_name, const std::string& buffer)
{
    std::string::size_type i = 0;
    std::ofstream fs(resource_cpp_filepath.c_str());

    if (!fs)
    {
        fprintf(stderr, "open %s error: %m\n", resource_cpp_filepath.c_str());
        return false;
    }

    // 写文件头
    fs << "// DO NOT EDIT!!!" << std::endl;
    fs << "// this file is auto generated by resource_maker" << std::endl;
    fs << "// edit the generator if necessary" << std::endl;
    fs << "#include <stdio.h> // size_t" << std::endl;
    fs << std::endl;
    fs << "namespace resource { // namespace resource BEGIN" << std::endl;
    fs << std::endl;

    // 记得减去结尾符
    fs << "    " << "size_t " << c_variable_name << "_size = " << buffer.size() - 1 << ";" << std::endl;
    fs << "    " << "unsigned char " << c_variable_name << "[] = {" << std::endl;

    while (true)
    {
        for (int j=0; j<16 && i<buffer.size()-1; ++j, ++i)
        {
            if (0 == j)
            {
                fs << "    ";
                fs << "    ";
            }

            fs << dec2hex(static_cast<unsigned char>(buffer[i]));
            if (i < buffer.size() - 2)
                fs << ",";
        }

        fs << std::endl;
        if (i == buffer.size()-1)
        {
            break;
        }
    }

    // 缩进4个空格
    fs << "    " << "};" << std::endl;

    // 写文件尾巴
    fs << std::endl;
    fs << "} // namespace resouce END" << std::endl;
    return true;
}

std::string dec2hex(unsigned char c)
{
    char buf[2+2+1]; // 第一个2为前缀0x，第二个2为内容，第三个1为结尾符
    snprintf(buf, sizeof(buf), "0x%02x", c); // 注意c类型如果为char，则需要强制转换成unsigned类型
    return buf;
}
