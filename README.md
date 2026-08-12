# UpdateDownloader
simple C++ learning  

该项目模拟软件更新：当检测不到MyApp.exe时，下载；当检测到MyApp.exe版本号低于服务器中的版本号时，下载更新，并备份旧版本MyApp.exe。完成后启动MyApp.exe。

./目录下是UpdateDownloader的源代码，./test_server/目录下是pyghon本地模拟的服务器（用于测试）下存放的用于更新的MyApp.exe版本号（.json）以及对应程序。./test_server/MyApp/目录下是MyApp的源代码。

./test_server/readme存放了一些简单的命令，方便该程序测试。
