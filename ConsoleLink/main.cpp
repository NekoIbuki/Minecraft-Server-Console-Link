#include "ConsoleLink.h"
#include <memory>
#include "outputMSG_API.h"
//#include "ConsoleLink.cpp"

namespace {
    unique_ptr<OutputMessageImpl> outputInstance;
}

// 获取当前时间戳
string GetTimestamp() {
    time_t now = time(nullptr);
    char buf[80];
    strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S]", localtime(&now));
    return buf;
}

HANDLE hStdinWritePipe; // 用于向子进程发送指令
atomic<bool> g_ServerRunning(true);
HANDLE g_hprocess;
ofstream g_chatLog, g_connLog, g_cmdLog, g_errLog, g_rawLog;

// 初始化日志文件
void InitLogFiles() {
    g_chatLog.open("chat.log", ios::app);//聊天
    g_connLog.open("connections.log", ios::app);//加入游戏信息
    g_cmdLog.open("commands.log", ios::app);//命令执行信息
    g_errLog.open("errors.log", ios::app);//报错信息
    g_rawLog.open("server_raw.log", ios::app);//服务器启动运行信息
}

// 写入日志文件
void WriteLog(ofstream& logFile, const string& content) {

    if (logFile.is_open()) {
        logFile << GetTimestamp() << " " << content << endl;
    }
}

//读取用户输入发送到子进程
void sendCommandServer() {
    string serverCommand;
    while (true)
    {
        getline(cin, serverCommand);
        if (serverCommand == "exit") break;//输入exit退出(临时
        serverCommand += "\n";
        DWORD bytesWritten;
        WriteFile(
            hStdinWritePipe,
            serverCommand.c_str(),//命令输入内容
            (DWORD)serverCommand.size(),//命令输入长度
            &bytesWritten,
            NULL
        );
    }
}

//路径检查
bool pathExists(const string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

bool isValidPath(const string& path, bool checkExistence = true) {
    // 检查空路径
    if (path.empty()) {
        cerr << COLOR_RED << "Error：路径不能为空" << COLOR_RESET << endl;
        system("pause");
        return 1;
    }

    // 检查完整路径格式
    regex pathRegex(R"(^([a-zA-Z]:)?([\\/][^\\/:\*\?"<>\|]+)*[\\/]?$)");
    if (!regex_match(path, pathRegex)) {
        cerr << COLOR_RED << "Error：路径格式不正确" << endl;
        cerr << "您输入的路径: " << path << endl;
        cerr << "有效路径示例:" << endl;
        cerr << "  C:\\Users\\Name" << COLOR_RESET << endl;
        system("pause");
        return 1;
    }

    //检查路径是否存在
    if (checkExistence && !pathExists(path)) {
        cerr << COLOR_RED << "Error：路径不存在 - " << COLOR_RESET << path << endl;
        system("pause");
        return 1;
    }

    return 0;
}

//检查path配置文件是否存在
bool fileExists(const string& filename) {
    ifstream f(filename);
    return f.good();
}
//Path配置文件创建
void createConfigIfNotExists(const string& filename, DefaultConfig& Config) {
    if (!fileExists(filename)) {
        ofstream outFile(filename);
        if (outFile) {
            // 添加前缀的配置格式
            outFile << "work_dir=\"" << Config.getDefaultPath() << "\"\n";
            outFile << "start_cmd=\"" << Config.getDefaultCmd() << "\"\n";
            cout << "配置文件 " << filename << " 已创建\n";
            cout << COLOR_GREEN << "默认工作路径设置为: " << COLOR_RESET << Config.getDefaultPath() << endl;
            cout << COLOR_GREEN << "默认启动参数为: " << COLOR_RESET << Config.getDefaultCmd() << endl;
        }
        else {
            cerr << COLOR_RED << "Error: 无法创建配置文件 " << COLOR_RESET << filename << endl;
            system("pause");
        }
    }
}
//string转wstring
wstring stringToWide(const string& str) {
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

// 从配置文件读取工作路径
string readWorkPathFromConfig(const string& filename) {
    ifstream inFile(filename);
    string line;

    while (getline(inFile, line)) {
        // 查找work_dir=开头的行
        if (line.find("work_dir=") == 0) {
            // 提取引号内的路径
            size_t start = line.find('\"');
            size_t end = line.rfind('\"');
            if (start != string::npos && end != string::npos && start < end) {
                return line.substr(start + 1, end - start - 1);
            }
        }
    }

    return ""; // 未找到返回空字符串
}

//读取启动参数
string readCommandFromConfig(const string& filename) {
    ifstream inFile(filename);
    string line;

    while (getline(inFile, line)) {
        // 查找start_cmd=开头的行
        if (line.find("start_cmd=") == 0) {
            // 提取引号内的路径
            size_t start = line.find('\"');
            size_t end = line.rfind('\"');
            if (start != string::npos && end != string::npos && start < end) {
                return line.substr(start + 1, end - start - 1);
            }
        }
    }
    return "";
}
//读取控制台输出
void ReadOutput(HANDLE hPipe) {
    char buffer[4096];
    DWORD bytesRead;
    while (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
        if (bytesRead == 0) break;
        cout << "[MC] " << string(buffer, bytesRead);
    }
}

// 处理服务端输出
void ProcessOutput(const string& line) {
    // 原始日志存储
    WriteLog(g_rawLog, line);

    //SharedMessage* pMsg = GetSharedMessage();            ///

    static OutputMessageImpl instance;//输出消息
    static regex joinBaseRegex(R"(\[\d+:\d+:\d+\] \[[^\]]+\] \[minecraft/MinecraftServer\]: (\w+) joined the game)");
    // 匹配详细登录信息
    static regex loginDetailRegex(R"(\[\d+:\d+:\d+\] \[[^\]]+\] \[minecraft/PlayerList\]: (\w+)\[/([\w:]+):\d+\] logged in with entity id \d+ at \(([\d.-]+), ([\d.-]+), ([\d.-]+)\))");
    // 匹配插件加载信息
    static regex pluginLoadRegex(R"(\[\d+:\d+:\d+\] \[[^\]]+\] \[([\w./]+)\]: .*was loaded)");

    smatch match;
    if (regex_search(line, match, joinBaseRegex)) {
        string msg = "[Join] " + match[1].str() + " joined the game";
        cout << COLOR_BLUE << msg << COLOR_RESET << endl;
        WriteLog(g_connLog, msg);
        instance.SetPCONN(msg); // 输出玩家连接信息

    }
    else if (regex_search(line, match, loginDetailRegex)) {
        string msg = "[Login] " + match[1].str() +
            " IP: " + match[2].str() +
            " Position: (" + match[3].str() + ", " +
            match[4].str() + ", " + match[5].str() + ")";
        cout << COLOR_CYAN << msg << COLOR_RESET << endl; // 青色显示详细信息
        WriteLog(g_connLog, msg);
    }
    else if (regex_search(line, match, pluginLoadRegex)) {
        string msg = "[Plugin] " + match[1].str() + " loaded";
        cout << COLOR_PURPLE << msg << COLOR_RESET << endl; // 紫色显示插件信息
        WriteLog(g_connLog, msg);
    }

    // 玩家聊天信息（绿色）
    regex chatRegex(R"(\[\d+:\d+:\d+\] \[[^\]]+\] \[minecraft/MinecraftServer\]: \[Not Secure\] <(\w+)> (.*))");
    smatch chatMatch;
    if (regex_search(line, chatMatch, chatRegex)) {
        string msg = "[Chat] " + chatMatch[1].str() + ": " + chatMatch[2].str();
        cout << COLOR_GREEN << msg << COLOR_RESET << endl;
        WriteLog(g_chatLog, msg);
        instance.SetPChat(msg); // 输出聊天信息
        //pMsg->type = MessageType::CHAT;
        //strcpy_s(pMsg->data.chatMsg, msg.c_str());        //
        return;
    }

    // 玩家执行命令（黄色）
    regex playerCmdRegex(R"(\[Not Secure\] \[(\w+)\] (.*))");
    smatch playerCmdMatch;
    if (regex_search(line, playerCmdMatch, playerCmdRegex)) {
        string msg = "[Player Command] " + playerCmdMatch[1].str() +
            ": " + playerCmdMatch[2].str();
        cout << COLOR_YELLOW << msg << COLOR_RESET << endl;
        WriteLog(g_cmdLog, msg);
        instance.SetPCMD(msg); // 输出玩家命令信息
        //pMsg->type = MessageType::COMMAND;
        //strcpy_s(pMsg->data.errorMsg, msg.c_str());        //
        return;
    }
    static const regex setTimeRegex(R"(\[(\w+): Set the time to (\d+)\])");
    if (regex_search(line, playerCmdMatch, setTimeRegex)) {
        string player = playerCmdMatch[1].str();
        string timeValue = playerCmdMatch[2].str();

        // 控制台输出（紫色显示）
        string consoleMsg = "[TIME SET] " + player + " → " + timeValue;
        cout << COLOR_PURPLE << consoleMsg << COLOR_RESET << endl;
        instance.SetPCMD(consoleMsg); // 输出玩家命令信息(Time)
        // 写入命令日志文件
        string logMsg = player + " set time to " + timeValue;
        WriteLog(g_cmdLog, logMsg);
        //pMsg->type = MessageType::COMMAND;
        //strcpy_s(pMsg->data.errorMsg, logMsg.c_str());      //
        return;
    }

    // 命令方块执行（黄色）
    regex cmdBlockRegex(R"(\[minecraft/MinecraftServer\]: \[@: (.*)\])");
    smatch cmdBlockMatch;
    if (regex_search(line, cmdBlockMatch, cmdBlockRegex)) {
        string msg = "[Command Block] " + cmdBlockMatch[1].str();
        cout << COLOR_YELLOW << msg << COLOR_RESET << endl;
        WriteLog(g_cmdLog, msg);
        instance.SetBCMD(msg); // 输出命令方块信息
        //pMsg->type = MessageType::COMMAND;
        //strcpy_s(pMsg->data.errorMsg, msg.c_str());   //
        return;
    }

    static const regex atCommandRegex(
        //R"(\[\d+:\d+:\d+\] \[[^\]]+\] \[minecraft/MinecraftServer\]: \[Not Secure\] \[@\] (\w+) (.*))"
        R"(\[\d+:\d+:\d+\] \[[^\]]+\] \[minecraft/MinecraftServer\]: \[Not Secure\] \[@\] (.*))"
    );

    if (regex_search(line, cmdBlockMatch, atCommandRegex)) {
        string player = cmdBlockMatch[1].str();
        string command = cmdBlockMatch[2].str();

        // 格式化输出
        string consoleMsg = "[@CMD] " + player + ": " + command;
        string logMsg = "[@Command] " + player + " executed: " + command;

        // 控制台输出（亮黄色+粗体）
        cout << COLOR_YELLOW << consoleMsg << COLOR_RESET << endl;
        instance.SetBCMD(consoleMsg); // 输出命令方块信息
        //pMsg->type = MessageType::COMMAND;
        //strcpy_s(pMsg->data.errorMsg, consoleMsg.c_str());    //

        // 写入命令日志文件
        WriteLog(g_cmdLog, logMsg);
    }

    // 错误信息（红色）
    if (line.find("ERROR") != string::npos ||
        line.find("Exception") != string::npos ||
        line.find("WARN") != string::npos) {
        cerr << COLOR_RED << "[Error] " << line << COLOR_RESET << endl; // 红色显示
        WriteLog(g_errLog, "[Error] " + line);
		string msg = "[Error] " + line;
        instance.SetSERROR(msg);//输出报错信息

        //pMsg->type = MessageType::SError;
        //strcpy_s(pMsg->data.errorMsg, msg.c_str());       //
        return;
    }


    // 默认输出（原样显示）
    cout << line << endl;
}

int main()
{
    //// 检查DLL是否加载成功
    //HMODULE hDll = LoadLibrary(TEXT("OUTMSGLink_DLL.dll"));
    //if (!hDll) {
    //    std::cerr << "DLL加载失败，错误代码: " << GetLastError() << std::endl;
    //    return 1;
    //}
    //std::cout << "DLL已成功加载！" << std::endl;

    //// 获取全局实例地址（需在DLL中导出此函数）
    //auto GetInstanceFunc = reinterpret_cast<void* (*)()>(
    //    GetProcAddress(hDll, "GetGlobalOutputMessage"));
    //if (!GetInstanceFunc) {
    //    std::cerr << "无法获取函数地址，错误代码: " << GetLastError() << std::endl;
    //    FreeLibrary(hDll);
    //    return 1;
    //}
    //std::cout << "全局实例地址: " << GetInstanceFunc() << std::endl;

    outputInstance = make_unique<OutputMessageImpl>();

    //路径文本读取文件
    const string configFile = "server_config.txt";

    //创建配置文件
    DefaultConfig Config;
    createConfigIfNotExists(configFile, Config);

    //从配置文件读取工作路径
    string workPath = readWorkPathFromConfig(configFile);
    string workCommand = readCommandFromConfig(configFile);
    //宽字符转换
    wstring serverDir = stringToWide(workPath);
    wstring command = stringToWide(workCommand);
    //
    if (!workCommand.empty())
    {
        cout << COLOR_BLUE << "从配置文件读取到的启动配置: " << COLOR_RESET << workCommand << endl;
    }
    if (!workPath.empty()) {
        cout << COLOR_BLUE << "从配置文件读取的工作路径: " << COLOR_RESET << workPath << endl;

        // 验证路径有效性
        if (isValidPath(workPath)) {
            cerr << COLOR_RED << "Error: 配置的工作路径无效" << COLOR_RESET << endl;
            system("pause");
            return 1;
        }
    }
    else {
        cerr << COLOR_RED << "Error: 未能在配置文件中找到有效的工作路径" << COLOR_RESET << endl;
        system("pause");
        return 1;
    }

    InitLogFiles(); // 初始化日志文件

    // 创建输出管道并启动进程
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        cerr << COLOR_RED << "CreatePipe failed: " << GetLastError() << COLOR_RESET << endl;
        return 1;
    }

    SetHandleInformation(hWritePipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);//子进程写入端句柄
    //创建输入管道
    HANDLE hStdinReadPipe;
    CreatePipe(&hStdinReadPipe, &hStdinWritePipe, &sa, 0);
    SetHandleInformation(hStdinReadPipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hStdinReadPipe;   // 重定向输入
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;

    PROCESS_INFORMATION pi;

    BOOL success = CreateProcess(
        NULL,
        &command[0],//cmd命令
        NULL,
        NULL,
        TRUE,
        CREATE_NEW_CONSOLE,//两个控制台窗口,0共用一个窗口
        //CREATE_NO_WINDOW,//隐藏服务端控制台窗口
        NULL,
        serverDir.c_str(),  // 使用用户输入的路径
        &si,
        &pi
    );

    if (!success) {
        cerr << COLOR_RED << "Failed to start server! Error: " << GetLastError() << COLOR_RESET << endl;
        return 1;
    }
    cout << COLOR_GREEN << "进程已启动 (PID: " << COLOR_RESET << pi.dwProcessId << ")" << endl;

    //关闭不必要句柄
    CloseHandle(hWritePipe);
    CloseHandle(hStdinReadPipe);

    //监听用户输入
    thread inputThread(sendCommandServer);
    inputThread.detach();

    //启动输出读取线程
    //thread(ReadOutput, hReadPipe).detach();

    //同步读取输出

    char buffer[4096];
    DWORD bytesRead;
    while (g_ServerRunning && ReadFile(hReadPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
        if (bytesRead > 0) {
            std::string output(buffer, bytesRead);
            ProcessOutput(output); // 处理并存储输出
        }
    }

    // 关闭日志文件
    g_chatLog.close();
    g_connLog.close();
    g_cmdLog.close();
    g_errLog.close();
    g_rawLog.close();

    //等待进程结束
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hReadPipe);
    system("pause");
    return 0;
}