#!/bin/sh
# Writed by yijian on 2020/9/6
# 查找被删除但仍然占据磁盘的文件

dirs=(`ls -l --time-style=long-iso /proc  | awk '{ print $8 }'`)
for ((i=0; i<${#dirs[@]}; ++i))
do
  dir=${dirs[i]}
  if test -z "$dir"; then
    continue
  fi
  $(expr $dir + 0 > /dev/null 2>&1)
  if test $? -ne 0; then
    continue
  fi

  pid=$dir
  lsof -p $pid | grep "deleted"
done
