## make与cmake的用法与区别
make：是执行构建的 “工人” —— 读取Makefile文件中的指令，执行编译、链接等具体操作，完成代码到可执行程序的转换。
cmake：是生成 Makefile 的 “设计师” —— 不直接编译代码，而是通过跨平台的CMakeLists.txt配置文件，自动生成适配不同系统（Linux/macOS/Windows）的Makefile（或 Visual Studio 工程文件），让make能直接使用。