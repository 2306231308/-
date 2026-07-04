// ==========================================
// chat.h —— Chat 类的声明
// ==========================================
// 这个文件只声明"有什么"，不写"怎么做"。
// 具体实现写在 chat.cpp 里。
//
// 使用方法（在 main.cpp 里）：
//   #include "chat.h"
//   Chat chat("你的API密钥", "你是一个助手");
//   string reply = chat.ask("你好");
// ==========================================

#ifndef CHAT_H
#define CHAT_H

#include <string>
#include <stack>

using namespace std;

struct Message {
    string role;
    string content;
    Message* prev;
    Message* next;
    Message(string r, string c) : role(r), content(c), prev(nullptr), next(nullptr) {}
};

class MessageList {
public:
    MessageList() : head(nullptr), tail(nullptr), size(0) {}
    ~MessageList();
    void push_back(string role, string content);
    void pop_back();
    void clear();
    int getSize() const;
    Message* getHead() const;
    Message* getTail() const;
    Message* getAt(int index) const;

private:
    Message* head;
    Message* tail;
    int size;
};

class Chat {
public:
    // ---------- 公开接口：使用者只需要看这部分 ----------

    // 构造函数：创建对象时传入 API 密钥和系统提示词
    Chat(string apiKey, string systemPrompt);

    // 析构函数：对象销毁时自动调用，清理资源
    ~Chat();

    // 发送一条消息，返回 AI 回复（失败时返回空字符串）
    string ask(string userInput);

    // 清空对话历史（系统提示词会保留）
    void clear();

    // 查看当前消息数（含系统提示词）
    int messageCount();

private:
    // ---------- 内部数据：外面访问不到 ----------
    string apiKey;                  // API 密钥
    string systemPrompt;            // 系统提示词
    MessageList messages;           // 消息历史（双向链表）

    // ---------- 内部辅助函数 ----------
    void addMessage(string role, string content);  // 添加一条消息
    string escapeJson(const string& s);             // JSON 特殊字符转义
    string historyToJson();                         // 历史打包成 JSON
    string callApi(string messages);                // 调用 API
    string extractReply(string response);           // 解析 AI 回复
    int kmpSearch(const string& text, const string& pattern);  // KMP 字符串匹配
    void buildKmpTable(const string& pattern, int* table);     // 构建 KMP 前缀表
    string reverseMessages();                       // 反转消息历史（栈应用）
};

#endif
