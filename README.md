# Simple File System
A simple fie system based on FAT.

运行代码的说明: 
```shell script
# 首先进入代码文件夹中
mkdir "build"   # 创建文件夹用于存储可执行文件
cd build        # 进入文件夹中
cmake ..        # 生成编译指令
make            # 编译程序
./fs            # 运行程序
```
使用上述方法，完成程序启动
程序其他操作的说明：

```shell script
init # 挂在磁盘（共享内存）在每次运行程序时，必须运行该命令
format # 格式化磁盘，在第一次运行程序时，必须运行该命令，在后续运行程序时，使用改名会清空磁盘上的所有数据
help # 输出帮助信息
ls # 显示当前文件夹中的文件
cd dirname # 前往一个文件夹，目标文件夹必须是父目录或在当前文件夹中。
mkdir dirname # 创建一个文件夹
touch filename # 创建一个文件，初始化它的内容为空字符
write filename [w|a] # 写文件，write filename w 代表清空文件中所有数据，从头写入，write filename a 代表以附加的方式写入文件
rm filename # 删除文件
rmdir dirname # 删除文件夹。其中文件夹必须为空
cat filename # 输出文件中的内容
chname old new # 将 old 文件/文件夹的名字改为 new
```