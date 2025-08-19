#pragma once
#include <string>
#include <mutex>
#include <windows.h>

#ifdef OUTPUTMSG_EXPORTS
#define OUTPUTMSG_API __declspec(dllexport)
#else
#define OUTPUTMSG_API __declspec(dllimport)
#endif

// 纯虚接口类
class OUTPUTMSG_API IOutputMessage {
public:
    virtual ~IOutputMessage() = 0;

    virtual std::string GetPChat() const = 0;
    virtual std::string GetPCONN() const = 0;
    virtual std::string GetPCMD() const = 0;
    virtual std::string GetBCMD() const = 0;
    virtual std::string GetSERROR() const = 0;
};

inline IOutputMessage::~IOutputMessage() = default;

class OutputMessageImpl;

void ProcessOutput(const std::string& line);

class OUTPUTMSG_API OutputMessageImpl : public IOutputMessage {
public:
   /* OutputMessageImpl() = default;
    ~OutputMessageImpl() override = default;*/
    OutputMessageImpl();
    ~OutputMessageImpl();


    // 接口实现
    std::string GetPChat() const override;
    std::string GetPCONN() const override;
    std::string GetPCMD() const override;
    std::string GetBCMD() const override;
    std::string GetSERROR() const override;

private:

    struct Impl;
    Impl* pImpl;

    friend void ::ProcessOutput(const std::string& line);

    void SetPChat(const std::string& msg);
    void SetPCONN(const std::string& msg);
    void SetPCMD(const std::string& msg);
    void SetBCMD(const std::string& msg);
    void SetSERROR(const std::string& msg);

    // 线程安全数据
    //mutable std::mutex mutex_;

};

extern "C" {
    OUTPUTMSG_API const IOutputMessage& GetGlobalOutputMessage();
}
