#!/bin/bash

# 获取当前脚本所在目录的路径
current_path=$(dirname $(realpath $0))
# 获取当前路径的上一级路径
parent_path=$(dirname "$current_path")

# 要替换的路径
old_path='/home/gejingquan/projects'
new_path="$parent_path"

# 文件路径
list_file='boothotpatch_list.txt'
source_file='boothotpatch_monitor.c'
service_file='load-boothotpatch-ko.service'

# 检查 boothotpatch_list.txt 文件是否存在
if [ -f "$list_file" ]; then
    # 检查并替换 boothot_list.txt 文件第五行中的路径
    if [ $(sed -n '5p' "$list_file" | grep -c "$old_path") -gt 0 ]; then
        sed -i "5s|$old_path|$new_path|g" "$list_file"
        echo "Replaced path in $list_file."
    fi
else
    echo "File $list_file does not exist!"
fi

# 检查 boothotpatch_monitor.c 文件是否存在
if [ -f "$source_file" ]; then
    # 使用 sed 替换特定行中的路径
    sed -i "33s|$old_path|$new_path|; 86s|$old_path|$new_path|; 92s|$old_path|$new_path|; 132s|$old_path|$new_path|" "$source_file"
    echo "Replaced paths in $source_file."
else
    echo "File $source_file does not exist!"
fi

# 获取当前内核版本字符串
kernel_version=$(uname -r)

# 检查 load-boothotpatch-ko.service 文件是否存在
if [ -f "$service_file" ]; then
    # 使用 sed 替换第7行中的版本号
    sed -i "7s|5.15.0-1046-raspi|$kernel_version|" "$service_file"
    echo "Replaced version in $service_file with kernel version: $kernel_version"
else
    echo "File $service_file does not exist!"
fi
