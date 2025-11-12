#include <cstdio>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdint>

// 1. 被Hook的函数（最简单的无参函数）
void my_func() {
    printf("原函数执行\n");
}

// 2. 钩子函数（不调用原函数，仅验证是否被触发）
void hooked_func() {
    printf("[Hook] 钩子函数被调用\n");  // 只打印日志，不调用原函数
}

// 3. 安装钩子的函数
void install_hook() {
    void* target = (void*)my_func;    // 目标函数地址
    void* hook = (void*)hooked_func;  // 钩子函数地址

    // 保存原函数前5字节
    uint8_t original_bytes[5];
    memcpy(original_bytes, target, 5);

    // 计算内存页并修改权限（允许修改原函数入口）
    size_t page_size = sysconf(_SC_PAGE_SIZE);
    uintptr_t target_page = (uintptr_t)target & ~(page_size - 1);
    mprotect((void*)target_page, page_size, PROT_READ | PROT_WRITE | PROT_EXEC);

    // 构造跳转指令（跳转到钩子函数）
    uint8_t jump_code[5] = {0xE9};  // jmp指令
    int32_t offset = (int32_t)((uintptr_t)hook - (uintptr_t)target - 5);
    memcpy(&jump_code[1], &offset, 4);

    // 覆盖原函数入口（写入跳转指令）
    memcpy(target, jump_code, 5);

    // 恢复内存权限
    mprotect((void*)target_page, page_size, PROT_READ | PROT_EXEC);
}

int main() {
    install_hook();
    my_func();  // 调用被Hook的函数，预期触发钩子
    return 0;
}