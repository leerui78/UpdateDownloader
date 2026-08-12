# UpdateDownloader
simple C++ learning  

该项目模拟软件更新：当检测不到MyApp.exe时，下载；当检测到MyApp.exe版本号低于服务器中的版本号时，下载更新，并备份旧版本MyApp.exe。完成后启动MyApp.exe。

./目录下是UpdateDownloader的源代码，./test_server/目录下是pyghon本地模拟的服务器（用于测试）下存放的用于更新的MyApp.exe版本号（.json）以及对应程序。./test_server/MyApp/目录下是MyApp的源代码。

./test_server/readme存放了一些简单的命令，方便该程序测试。

1. 首先开启pyghon本地模拟的服务器；
2. 运行UpdateDownloader.exe
<img width="897" height="454" alt="image" src="https://github.com/user-attachments/assets/9f425160-d12e-4bbb-b55c-ef43d02e55bc" />
3. 点击检查更新
<img width="1064" height="452" alt="image" src="https://github.com/user-attachments/assets/7c9cd4aa-1c52-4a45-aa5a-1b6cdb02d0a5" />
4. 点击ok，进入MyApp
<img width="1199" height="852" alt="image" src="https://github.com/user-attachments/assets/b0bc3e89-808a-4543-8d9b-fe450f4b68bd" />
