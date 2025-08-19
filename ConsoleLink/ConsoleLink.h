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

// ����̨��ɫ����
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32;1m"
#define COLOR_YELLOW "\033[33;1m"
#define COLOR_BLUE   "\033[34;1m"
#define COLOR_CYAN "\033[36m"
#define COLOR_PURPLE "\033[35m"
#define COLOR_RESET  "\033[0m"

using namespace std;

//Ĭ�ϲ���
class DefaultConfig {
public:
    string defaultPath; // Ĭ�Ϲ���·��
    string defaultCmd;  // Ĭ����������
    DefaultConfig() {
        defaultPath = "C:\\Users\\Projects"; // Ĭ�Ϲ���·��
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