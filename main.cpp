// ==========================================
// main.cpp —— 使用 Chat 类进行多轮对话
// ==========================================
// 编译方法（要把 chat.cpp 一起编译进来）：
//   g++ main.cpp chat.cpp -lcurl -o chat
// 然后运行：
//   ./chat
// ==========================================

#include <windows.h>
#include "chat.h"
#include <iostream>
#include <string>

using namespace std;

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    // ========== 1. 配置 API 密钥 ==========
    // 在 https://platform.deepseek.com 注册获取
    string apiKey = "sk-1780d29476644cfe9749bfa8e29045c7";

    // ========== 2. 创建聊天对象 ==========
    // 第一个参数：API 密钥
    // 第二个参数：系统提示词（定义 AI 的性格和回答风格）
    Chat chat(apiKey, "你是一个乐于助人的AI助手，用中文回答，回答要简洁明了。");

    cout << "\n===== DeepSeek 多轮对话机器人 =====" << endl;
    cout << "输入 quit 退出，输入 clear 清空历史" << endl;
    cout << "==================================\n" << endl;

    // ========== 3. 循环聊天 ==========
    string userInput;
    while (true) {
        cout << "你: ";
        getline(cin, userInput);

        if (userInput == "quit" || userInput == "exit") {
            break;
        }

        if (userInput == "clear") {
            chat.clear();
            cout << "对话历史已清空" << endl;
            continue;
        }

        // 调用类的 ask 方法，一句话完成"发送+接收"
        string reply = chat.ask(userInput);

        if (reply.empty()) {
            cout << "请求失败，请检查网络或 API 密钥" << endl;
            continue;
        }

        cout << "AI: " << reply << endl << endl;
    }

    cout << "再见！" << endl;
    return 0;
}
