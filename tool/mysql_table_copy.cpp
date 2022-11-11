// Writed by yijian on 2019/4/30
//
// 将一个表的数据从一个数据库复制到另一个数据库的工具，
// 数据源通过参数“--sql”指定，可带条件和LIMIT等，
// 目标字段可通过参数“--dfields”指定，也可以不指定。
// 成功则返回0，出错返回1，没数据返回2。
// 失败信息在标准出错上输出，成功信息在标准输出上输出，
// 失败信息标识为“FAILED”，成功信息标识为“SUCCESS”。
// 如果数据不为空，则在成功标识“SUCCESS”后紧跟第一个字段的最新值，
// 如果这是一个自增字段值，则可借助这个值实现增量复制。
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/signal_handler.h>
#include <mooon/sys/stop_watch.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/print_color.h>
#include <mooon/utils/string_utils.h>

// Source database
STRING_ARG_DEFINE(shost, "127.0.0.1", "Source database host, example: --shost=127.0.0.1");
INTEGER_ARG_DEFINE(uint16_t, sport, 3306, 1000, 65535, "Source database port, example: --sport=3306");
STRING_ARG_DEFINE(sname, "", "Source database name, example: --sname=test");
STRING_ARG_DEFINE(suser, "", "Source database user, example: --suser=root");
STRING_ARG_DEFINE(spassword, "", "Source database password, example: --spassword='123456'");
STRING_ARG_DEFINE(sql, "", "The sql to query source table, example: --sql='SELECT * FROM test LIMIT 2019'");
INTEGER_ARG_DEFINE(int, sconntimeout, 10, 0, 600, "Connect source database timeout in seconds");
INTEGER_ARG_DEFINE(int, sreadtimeout, 10, 0, 600, "Read source database timeout in seconds");

// Destination database
STRING_ARG_DEFINE(dhost, "127.0.0.1", "Destination database host, example: --dhost=127.0.0.1");
INTEGER_ARG_DEFINE(uint16_t, dport, 3306, 1000, 65535, "Source database port, example: --dport=3306");
STRING_ARG_DEFINE(dname, "", "Destination database name, example: --dname=test");
STRING_ARG_DEFINE(duser, "", "Destination database user, example: --duser=root");
STRING_ARG_DEFINE(dpassword, "", "Destination database password, example: --dpassword='123456'");
STRING_ARG_DEFINE(dtable, "", "Destination database table, example: --dtable=test");
STRING_ARG_DEFINE(dfields, "*", "Destination database table fields, example: --dfields=a,b,c,d");
INTEGER_ARG_DEFINE(int, dconntimeout, 10, 0, 600, "Connect destination database timeout in seconds");
INTEGER_ARG_DEFINE(int, dwritetimeout, 10, 0, 600, "Write destination database timeout in seconds");
INTEGER_ARG_DEFINE(int, dreadtimeout, 10, 0, 600, "Read destination database timeout in seconds");

// 默认只是测试，不实际写目标DB
BOOL_STRING_ARG_DEFINE(test, "false", "If true only test, will ignore all destination parameters exclude --dtable and --dfields, example: --test=true");

// 是否忽略插入时遇到的错误
BOOL_STRING_ARG_DEFINE(ignore, "false", "If true errors that occur while executing the INSERT statement are ignored, example: --ignore=true");

// 显示详细信息
BOOL_STRING_ARG_DEFINE(verbose, "false", "Displays runtime details, example: --verbose=true");

// 单次查询数据条数硬限制（0表示不限制）
INTEGER_ARG_DEFINE(int, hdlimit, 10000, 0, std::numeric_limits<int>::max(), "The number of records for a single query, example: --hdlimit=100000");

class CTableCopyer
{
public:
    int copy();

private:
    bool init();
    bool init_source_mysql();
    bool init_destination_mysql();
    void print_cost(_IO_FILE* stdxxx, mooon::sys::CStopWatch& stopwatch);

private:
    mooon::sys::CMySQLConnection _source_mysql;
    mooon::sys::CMySQLConnection _destination_mysql;
};

static void usage()
{
    fprintf(stderr, "Exit codes:\n");
    fprintf(stderr, "1) " PRINT_COLOR_YELLOW"Success" PRINT_COLOR_NONE" exits with 0.\n");
    fprintf(stderr, "2) " PRINT_COLOR_YELLOW"Error" PRINT_COLOR_NONE" exits with 1.\n");
    fprintf(stderr, "3) " PRINT_COLOR_YELLOW"No data" PRINT_COLOR_NONE" exits with 2.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
}

// 返回0成功，
// 返回1出错，
// 返回2表示没数据
int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        if (!errmsg.empty())
            fprintf(stderr, "%s.\n", errmsg.c_str());
        else
            fprintf(stderr, "\n");
        usage();
        return 1;
    }

    // --shost
    if (mooon::argument::shost->value().empty())
    {
        fprintf(stderr, "Parameter[--shost] is not set.\n\n");
        usage();
        return 1;
    }
    // --sname
    if (mooon::argument::sname->value().empty())
    {
        fprintf(stderr, "Parameter[--sname] is not set.\n\n");
        usage();
        return 1;
    }
    // --suser
    if (mooon::argument::suser->value().empty())
    {
        fprintf(stderr, "Parameter[--suser] is not set.\n\n");
        usage();
        return 1;
    }
    // --spassword
    if (mooon::argument::spassword->value().empty())
    {
        fprintf(stderr, "Parameter[--spassword] is not set.\n\n");
        usage();
        return 1;
    }
    // --sql
    if (mooon::argument::sql->value().empty())
    {
        fprintf(stderr, "Parameter[--sql] is not set.\n\n");
        usage();
        return 1;
    }
    if (mooon::argument::test->is_false())
    {
        // --dhost
        if (mooon::argument::dhost->value().empty())
        {
            fprintf(stderr, "Parameter[--dhost] is not set.\n\n");
            usage();
            return 1;
        }
        // --dname
        if (mooon::argument::dname->value().empty())
        {
            fprintf(stderr, "Parameter[--dname] is not set.\n\n");
            usage();
            return 1;
        }
        // --duser
        if (mooon::argument::duser->value().empty())
        {
            fprintf(stderr, "Parameter[--duser] is not set.\n\n");
            usage();
            return 1;
        }
        // --dpassword
        if (mooon::argument::dpassword->value().empty())
        {
            fprintf(stderr, "Parameter[--dpassword] is not set.\n\n");
            usage();
            return 1;
        }
    }
    // --dtable
    if (mooon::argument::dtable->value().empty())
    {
        fprintf(stderr, "Parameter[--dtable] is not set.\n\n");
        usage();
        return 1;
    }

    return CTableCopyer().copy();
}

// 返回0成功，
// 返回1出错，
// 返回2表示没数据
int CTableCopyer::copy()
{
    std::string tag;
    std::string querysql;
    std::string insertsql;
    mooon::sys::CStopWatch stopwatch;

    if (!init())
    {
        print_cost(stderr, stopwatch);
        fprintf(stderr, "FAILED\n");
        return 1;
    }
    try
    {
        const bool have_limit = strcasestr(mooon::argument::sql->c_value(), " LIMIT ") != NULL; // 是否已含有LIMIT
        std::vector<std::string> values;
        std::string values_str;
        std::string first_field;
        std::string ignore_str;
        mooon::sys::DBTable dbtable;

        // 防止单词SELECT数据量过大，强制加上LIMIT硬保护
        if (have_limit || mooon::argument::hdlimit->value()==0)
        {
            querysql = mooon::argument::sql->value();
        }
        else
        {
            querysql = mooon::utils::CStringUtils::format_string(
                    "%s LIMIT %d", mooon::argument::sql->c_value(), mooon::argument::hdlimit->value());
        }
        if (mooon::argument::verbose->is_true())
        {
            fprintf(stdout, "[SELECTSQL] %s\n", querysql.c_str());
        }

        tag = "SELECT_FROM_SOURCE";
        _source_mysql.query(dbtable, "%s", querysql.c_str());
        if (dbtable.empty())
        {
            fprintf(stderr, "[%s] NODATA\n", mooon::sys::CDatetimeUtils::get_current_datetime().c_str());
            return 2; // 方便调用者区分错误结束循环
        }
        for (mooon::sys::DBTable::size_type row=0; row<dbtable.size(); ++row)
        {
            const mooon::sys::DBRow& dbrow = dbtable[row];
            std::string value;

            for (mooon::sys::DBRow::size_type col=0; col<dbrow.size(); ++col)
            {
                if (col == 0)
                {
                    first_field = dbrow[col];
                }
                if (value.empty())
                    value = std::string("('") + dbrow[col] + std::string("'");
                else
                    value = value + std::string(",'") + dbrow[col] + std::string("'");
            }
            value = value + std::string(")");
            values.push_back(value);
        }

        values_str = mooon::utils::CStringUtils::container2string(values, ",");
        if (mooon::argument::ignore->is_true())
        {
            ignore_str = " IGNORE ";
        }
        else
        {
            ignore_str = " ";
        }
        if (mooon::argument::dfields->value().empty() || mooon::argument::dfields->value()=="*")
        {
            insertsql = mooon::utils::CStringUtils::format_string(
                    "INSERT%sINTO %s VALUES %s", ignore_str.c_str(),
                    mooon::argument::dtable->c_value(), values_str.c_str());
        }
        else
        {
            insertsql = mooon::utils::CStringUtils::format_string(
                    "INSERT%sINTO %s (%s) VALUES %s", ignore_str.c_str(),
                    mooon::argument::dtable->c_value(), mooon::argument::dfields->c_value(), values_str.c_str());
        }

        if (mooon::argument::test->is_true() ||
            mooon::argument::verbose->is_true())
        {
            if (insertsql.size() < mooon::SIZE_32K)
                fprintf(stdout, "[INSERTSQL] %s\n", insertsql.c_str());
            else
                fprintf(stdout, "[INSERTSQL] %.*s ... (%zu more than %d)\n",
                        mooon::SIZE_32K, insertsql.c_str(), insertsql.size(), mooon::SIZE_32K);
        }
        if (mooon::argument::test->is_false())
        {
            tag = "INSERT_INTO_DESTINATION";
            _destination_mysql.update("%s", insertsql.c_str());
        }
        if (first_field.empty())
        {
            print_cost(stdout, stopwatch);
            fprintf(stdout, "ROW: %zu\n", dbtable.size());
            fprintf(stdout, "SUCCESS: none\n");
        }
        else
        {
            print_cost(stdout, stopwatch);
            fprintf(stdout, "ROW: %zu\n", dbtable.size());
            // 如果 first_field 是自增 ID 字段，则增量扫描可以从该值（不包含）开始
            fprintf(stdout, "SUCCESS: %s (The latest value of the first field, %s)\n",
                    first_field.c_str(), mooon::sys::CDatetimeUtils::get_current_datetime().c_str());
        }
        return 0;
    }
    catch (mooon::sys::CDBException& ex)
    {
        if (tag.empty())
        {
            fprintf(stderr, "%s\n", ex.str().c_str());
        }
        else
        {
            if (tag == "SELECT_FROM_SOURCE")
                fprintf(stderr, "[SELECTSQL] %s\n", querysql.c_str());
            else if (tag == "INSERT_INTO_DESTINATION")
                fprintf(stderr, "[INSERTSQL] %s\n", insertsql.c_str());
            fprintf(stderr, "[%s] %s\n", tag.c_str(), ex.str().c_str());
        }

        print_cost(stderr, stopwatch);
        fprintf(stderr, "FAILED\n");
        return 1;
    }
}

bool CTableCopyer::init()
{
    mooon::sys::CSignalHandler::ignore_signal(SIGPIPE);
    return (init_source_mysql() && init_destination_mysql());
}

bool CTableCopyer::init_source_mysql()
{
    for (int i=0; i<3; ++i)
    {
        try
        {
            _source_mysql.set_null_value("");
            _source_mysql.set_host(mooon::argument::shost->value(), mooon::argument::sport->value());
            _source_mysql.set_db_name(mooon::argument::sname->value());
            _source_mysql.set_user(mooon::argument::suser->value(), mooon::argument::spassword->value());
            _source_mysql.enable_auto_reconnect(true);
            _source_mysql.set_connect_timeout_seconds(mooon::argument::sconntimeout->value());
            _source_mysql.set_read_timeout_seconds(mooon::argument::sreadtimeout->value());
            _source_mysql.open();
            return true;
        }
        catch (mooon::sys::CDBException& ex)
        {
            if (i == 1) {
                // 延迟第2次重试
                mooon::sys::CUtils::millisleep(1000);
            }
            else if (i == 2) {
                fprintf(stderr, "Intialize source MySQL failed: %s.\n", ex.str().c_str());
                break;
            }
        }
    }

    return false;
}

bool CTableCopyer::init_destination_mysql()
{
    for (int i=0; i<3; ++i)
    {
        try
        {
            if (mooon::argument::test->is_true())
            {
                return true;
            }
            else
            {
                _destination_mysql.set_host(mooon::argument::dhost->value(), mooon::argument::dport->value());
                _destination_mysql.set_db_name(mooon::argument::dname->value());
                _destination_mysql.set_user(mooon::argument::duser->value(), mooon::argument::dpassword->value());
                _destination_mysql.enable_auto_reconnect(true);
                _destination_mysql.set_connect_timeout_seconds(mooon::argument::dconntimeout->value());
                _destination_mysql.set_write_timeout_seconds(mooon::argument::dwritetimeout->value());
                _destination_mysql.set_read_timeout_seconds(mooon::argument::dreadtimeout->value());
                _destination_mysql.open();
                return true;
            }
        }
        catch (mooon::sys::CDBException& ex)
        {
            if (i == 1) {
                // 延迟第2次重试
                mooon::sys::CUtils::millisleep(1000);
            }
            else if (i == 2) {
                fprintf(stderr, "Intialize destination MySQL failed: %s.\n", ex.str().c_str());
                break;
            }
        }
    }

    return false;
}

void CTableCopyer::print_cost(_IO_FILE* stdxxx, mooon::sys::CStopWatch& stopwatch)
{
    const uint64_t microseconds = stopwatch.get_elapsed_microseconds();
    const double seconds = microseconds / (1000000.0);
    fprintf(stdxxx, "COST: %.3fs (%" PRIu64"us)\n", seconds, microseconds);
}
