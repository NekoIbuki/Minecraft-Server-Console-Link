#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <regex>
#include <locale>
#include <codecvt>
#include <thread>
#include <ctime>
#include <atomic>
#include <mutex>

// 控制台颜色设置
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32;1m"
#define COLOR_YELLOW "\033[33;1m"
#define COLOR_BLUE   "\033[34;1m"
#define COLOR_CYAN "\033[36m"
#define COLOR_PURPLE "\033[35m"
#define COLOR_RESET  "\033[0m"

using namespace std;

//默认参数
class DefaultConfig {
public:
    string defaultPath; // 默认工作路径
    string defaultCmd;  // 默认启动配置
    DefaultConfig() {
        defaultPath = "C:\\Users\\Projects"; // 默认工作路径
        defaultCmd =
            "java "
            "-Xms1G -Xmx1G "
            "win_args.txt";
    }
    string getDefaultPath() {
        return defaultPath;
    }
    string getDefaultCmd() {
        return defaultCmd;
    }
};

//class outputMessage {
//public:
//    string playerChat;
//    string playerConnect;
//    string playerCommand;
//    string blockCommand;
//    string serverERROR;
//
//    string GetPChat() {
//        return playerChat;
//    }
//    string GetPCONN() {
//        return playerConnect;
//    }
//    string GetPCMD() {
//        return playerCommand;
//    }
//    string GetBCMD() {
//        return blockCommand;
//    }
//    string GetSERROR() {
//        return serverERROR;
//    }
//};