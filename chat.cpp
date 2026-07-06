// ==========================================
// chat.cpp —— Chat 类的方法实现
// ==========================================
// 这个文件写的是 chat.h 中声明的"怎么做"。
//
// 声明在 .h 里，实现在 .cpp 里 —— 这是 C++ 的标准做法。
// 好处：别人用这个类时，看 .h 就知道有哪些功能，
//       .cpp 里的细节想看就看，不想看也不影响使用。
// ==========================================

#include "chat.h"

#include <iostream>
#include <curl/curl.h>
#include <unordered_map>
#include <algorithm>
using namespace std;

static size_t writeCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

// ==========================================
// MessageList 双向链表实现
// ==========================================
MessageList::~MessageList() {
    clear();
}

void MessageList::push_back(string role, string content) {
    Message* newNode = new Message(role, content);
    if (tail == nullptr) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }
    size++;
}

void MessageList::pop_back() {
    if (tail == nullptr) return;
    Message* temp = tail;
    tail = tail->prev;
    if (tail == nullptr) {
        head = nullptr;
    } else {
        tail->next = nullptr;
    }
    delete temp;
    size--;
}

void MessageList::clear() {
    Message* current = head;
    while (current != nullptr) {
        Message* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
    tail = nullptr;
    size = 0;
}

int MessageList::getSize() const {
    return size;
}

Message* MessageList::getHead() const {
    return head;
}

Message* MessageList::getTail() const {
    return tail;
}

Message* MessageList::getAt(int index) const {
    if (index < 0 || index >= size) return nullptr;
    Message* current = head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    return current;
}

Chat::Chat(string apiKey, string systemPrompt) {
    this->apiKey = apiKey;
    this->systemPrompt = systemPrompt;
    addMessage("system", systemPrompt);
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

Chat::~Chat() {
    curl_global_cleanup();
}

string Chat::ask(string userInput) {
    addMessage("user", userInput);

    string response = callApi(historyToJson());
    if (response.empty()) {
        messages.pop_back();
        return "";
    }

    string reply = extractReply(response);
    if (reply.empty()) {
        cout << "[原始响应] " << response << endl;
        messages.pop_back();
        return "";
    }

    addMessage("assistant", reply);
    return reply;
}

void Chat::clear() {
    messages.clear();
    addMessage("system", systemPrompt);
}

int Chat::messageCount() {
    return messages.getSize();
}

void Chat::addMessage(string role, string content) {
    messages.push_back(role, content);
}

string Chat::escapeJson(const string& s) {
    static const unordered_map<char, string> escapeMap = {
        {'"', "\\\""},
        {'\\', "\\\\"},
        {'\n', "\\n"},
        {'\r', "\\r"},
        {'\t', "\\t"}
    };
    string result;
    for (char c : s) {
        auto it = escapeMap.find(c);
        if (it != escapeMap.end()) {
            result += it->second;
        } else {
            result += c;
        }
    }
    return result;
}

string Chat::historyToJson() {
    string json = "[";
    Message* current = messages.getHead();
    bool first = true;
    while (current != nullptr) {
        if (!first) json += ",";
        first = false;
        json += "{\"role\":\"" + current->role +
                "\",\"content\":\"" + escapeJson(current->content) + "\"}";
        current = current->next;
    }
    json += "]";
    return json;
}

// 调用 DeepSeek API
string Chat::callApi(string messages) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    string response;
    struct curl_slist* headers = NULL;

    string requestBody = "{"
        "\"model\":\"deepseek-chat\","
        "\"messages\":" + messages + ","
        "\"stream\":false"
    "}";
    string authHeader = "Authorization: Bearer " + apiKey;

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.deepseek.com/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

void Chat::buildKmpTable(const string& pattern, int* table) {
    int n = pattern.length();
    table[0] = 0;
    int len = 0;
    for (int i = 1; i < n; ) {
        if (pattern[i] == pattern[len]) {
            table[i++] = ++len;
        } else {
            if (len != 0) {
                len = table[len - 1];
            } else {
                table[i++] = 0;
            }
        }
    }
}

int Chat::kmpSearch(const string& text, const string& pattern) {
    int n = text.length();
    int m = pattern.length();
    if (m == 0) return 0;

    int* table = new int[m];
    buildKmpTable(pattern, table);

    int i = 0, j = 0;
    while (i < n) {
        if (pattern[j] == text[i]) {
            i++;
            j++;
        }
        if (j == m) {
            delete[] table;
            return i - j;
        } else if (i < n && pattern[j] != text[i]) {
            if (j != 0) {
                j = table[j - 1];
            } else {
                i++;
            }
        }
    }
    delete[] table;
    return -1;
}

string Chat::reverseMessages() {
    stack<string> msgStack;
    Message* current = messages.getHead();
    while (current != nullptr) {
        msgStack.push(current->role + ":" + current->content);
        current = current->next;
    }
    string reversed;
    while (!msgStack.empty()) {
        reversed += msgStack.top() + "\n";
        msgStack.pop();
    }
    return reversed;
}

string Chat::extractReply(string response) {
    string key = "\"content\":\"";
    int start = kmpSearch(response, key);
    if (start == -1) return "";
    start += key.length();

    int end = start;
    while (end < (int)response.length()) {
        if (response[end] == '\\' && end + 1 < (int)response.length()) {
            end += 2;
            continue;
        }
        if (response[end] == '"') break;
        end++;
    }
    if (end >= (int)response.length()) return "";

    string content = response.substr(start, end - start);

    stack<char> charStack;
    for (int i = 0; i < (int)content.length(); i++) {
        if (content[i] == '\\' && i + 1 < (int)content.length()) {
            char nextChar = content[i + 1];
            switch (nextChar) {
                case 'n': charStack.push('\n'); break;
                case 't': charStack.push('\t'); break;
                case '"': charStack.push('"'); break;
                case '\\': charStack.push('\\'); break;
                default: charStack.push(content[i]); charStack.push(nextChar); break;
            }
            i++;
        } else {
            charStack.push(content[i]);
        }
    }

    string result;
    while (!charStack.empty()) {
        result += charStack.top();
        charStack.pop();
    }
    reverse(result.begin(), result.end());
    return result;
}