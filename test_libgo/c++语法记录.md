# 零碎知识点
## c++的基础类型
1. 普通类型，如int、char等等
2. 空类型，void,函数返回值，无类型指针（void* 可指向任意类型数据，需强制转换后使用）
3. 复合类型
    -指针类型
***特殊：函数指针 eg:int(*)(int)（函数指针，指向 “参数为 int、返回 int 的函数”）**
    -引用类型
    -数组类型
    -函数类型
## typedef与using的用法
主要是用于给一种类型取一种别名，主要针对那种比较复杂的类型，如函数指针
``` c++
eg:using IntVec = std::vector<int>; 用 IntVec 代替 std::vector<int>
```
## const关键字的各种用法（就近修饰，谁近谁就是常量）
主要看那边能被修饰，默认修饰左边，若左边元素不可以被修饰，则修饰右边，eg：char* const p，‘*’不能被修饰，所以修饰的是 p（指针），const得修饰实体（int、*这些都不是）
1. 核心作用是保护常量不被修改 eg:const int a=100
2. 难点：修饰指针
    第一种：const char* p，这是指向常量的指针，指针可变（指向其他东西），指向内容不可变
    第二种：char* const p，这是常量指针，指针不可变，只能指向一个地址，这个地址的内容可变
    第三种：const char* const p 这是指针和指针指向的变量都是常量，即都不可变
3. 难点：修饰引用&
``` c++
    eg:const exception& e
    这里的const修饰的是exception&,exception&代指exception类型的引用
```
4. 修饰类的成员函数
const 放在成员函数的参数列表后、函数体前（或声明后），表示该成员函数不会修改所属对象的任何非 mutable 成员变量，且只能调用其他 const 成员函数。
``` c++
    eg:int get_val() const;  // 声明：参数列表后加const
    定义时需保留const
```
## &关键字的各种用法（取地址符、引用和位与运算符）
1. 取地址符：作用与*刚好相反 eg：int num=100，int *p=&num，&num就是num的地址，p是指针，刚好指向地址
2. 引用：相当于原数据的别名，共享内存，常用于函数参数中，可以避免拷贝，同时可在函数内修改函数外内容
3. 位与运算符：二进制同'1'取'1'，否则取'0'
##size_t的作用
size_t是一种无符号整数类型，用于表述内存大小、数组长度等等
## 实例类的命名规则
1. 一般：小写_功能
2. 全局：g_小写_功能
3. 实例既有可能在栈（自动回收），也可能在堆（手动释放）堆：CLASS* class_my=new CLASS() 栈：CLASS class_my
## 类/结构体是否需要显示定义默认构造函数
``` c++
1. 需要无参创建类 eg：Student* arr = new Student[10]; 此时需要默认构造函数
2. 我不显示定义任何构造函数，则此时不需要显示定义默认构造函数，不过成员变量可能被初始化错误
3. 默认构造函数：没有参数 带参构造函数：有参数
4. eg：    Student(string n, int a, float s) : name(n), age(a), score(s) {}

    // 显式定义默认构造函数
    Student() : name("Unknown"), age(0), score(0.0f) {}
```
*** 一些c++新特性（ros2会用）**
## auto的用法
*** auto关键字是类型推导工具 ***
``` c++
1. 普通用法：auto x=100，推导基础类型
2. 推导复合类型：int x=100,auto& ref=x;//这里会推导成int&，但是&写出来
3. 推导STL容器/迭代器：用于推导迭代器很好用，auto it=arr.begin()等价于vector<int>::iterator it=arr.begin()
4. 推导函数返回值：ag：auto sum（）{}
5. 推导lambda表达式：
```
## lambda表达式
*** 匿名函数 ***
lambda 表达式（λ 表达式）是 C++11 引入的一种匿名函数（没有函数名的函数），可以就地定义、就地使用，核心用于简化代码（尤其是回调、算法参数、临时逻辑），避免编写独立的函数 / 函数对象。
基本结构：
``` c++
[capture](parameters) mutable -> return_type {
    // 函数体（执行逻辑）
};
```
1. [capture]	捕获列表：决定外部变量能否在 lambda 内使用（核心）	❌ 必选
   [this](){}//捕获this指针，用于在lambda内调用类的成员函数
2. (parameters)	参数列表：和普通函数的参数一致（类型、数量、默认值等）	✅ 可选
3. mutable	允许修改按值捕获的变量（默认按值捕获的变量是 const 的）	✅ 可选
4. -> return_type	返回值类型：编译器可自动推导（C++11 起），复杂场景需显式指定	✅ 可选
5. {}	函数体：执行逻辑	❌ 必选
*** 核心要点 ***
几种捕获方式，lambda可以访问外部作用域变量
1. 空捕获	[]	不捕获任何外部变量（仅能用参数和局部变量）
2. 按值捕获	[x, y]	捕获变量 x、y 的副本（lambda 内修改不影响外部）
3. 按引用捕获	[&x, &y]	捕获变量 x、y 的引用（lambda 内修改会影响外部）
4. 按值捕获所有	[=]	捕获所有外部变量的副本（默认 const，需 mutable 才能修改）
5. 按引用捕获所有	[&]	捕获所有外部变量的引用（修改会影响外部）
## 智能指针
使用条件：#include<memory>
智能指针本身在栈，指向（管理）的内容在堆上，因为只有堆上的内容才需要管理释放内存
作用：自动管理内存生命周期，避免内存泄漏、野指针、重复释放等问题
eg:std::make_shared<T>(args)
三种智能指针：
1. unique_ptr：独占指针，仅可有一个指针指向某个资源，离开作用域自动释放内存
2. shared_ptr: 带引用计数的共享智能指针，多个 shared_ptr 可指向同一资源，计数为 0 时自动释放资源。
3. weak_ptr:弱引用指针是 shared_ptr 的 “辅助工具”，指向 shared_ptr 管理的资源，但不增加引用计数，也不影响资源销毁，核心解决 shared_ptr 的循环引用问题。
## 函数包装器
#include<functional>
核心功能：统一调用函数的接口，例如自由函数，成员函数，lambda函数
eg:std::function<返回值类型(参数类型1, 参数类型2, ...)> 包装器对象
关键：bind的用法：
class Calc {
public:
    int mul(int a, int b) { // 非静态成员函数
        return a * b;
    }
    static int div(int a, int b) { // 静态成员函数
        return a / b;
    }
};
Calc obj;
    // 包装非静态成员函数：需用&取地址，且绑定对象
    function<int(int, int)> func1 = bind(&Calc::mul, &obj, placeholders::_1, placeholders::_2);
这里的placeholders::_1, placeholders::_2是指mul的两个参数不固定，要固定就直接换成一个值



## 堆内存和栈内存
                                栈内存（Stack）	                                                堆内存（Heap）
内存大小	        固定且较小（通常几 MB，如 Linux 默认 8MB），由系统限制	    动态且极大（理论上受物理内存 + 虚拟内存限制），GB 级可用
分配 / 释放效率	     极快（仅需移动栈指针 esp/ebp，无系统调用）	                较慢（需操作系统查找空闲内存块，涉及malloc/free/new/delete系统调用）
内存碎片	        无碎片（栈是连续的 “后进先出” 结构，释放按顺序）	         易产生碎片（多次分配 / 释放不同大小的块，导致内存不连续）
生命周期	        与作用域绑定（如函数内的局部变量，函数结束即销毁）	         与作用域无关（手动分配后，直到free/delete才销毁，可跨函数 / 模块）
存储内容	        局部变量、函数参数、返回地址、寄存器备份等	                动态创建的对象、大数组、跨作用域的数据（如类的成员指针指向的内容）
访问方式	        直接访问（编译器知道栈变量的偏移地址）	                    间接访问（通过指针 / 引用，需解引用）
初始化规则	        未初始化的栈变量是 “垃圾值”（随机内存数据）	                未初始化的堆变量也是垃圾值，但new可指定初始化（如new int(10)）
线程安全性	        栈是线程私有（每个线程有独立栈，互不干扰）	                堆是进程共享（所有线程可访问，需加锁保证线程安全）
底层实现	        基于 “栈指针” 的连续内存区域，遵循 LIFO（后进先出）	        基于 “空闲内存链表” 的离散区域，由操作系统内存管理器管理

## c++宏定义的作用
1. 文本替换：仅做字符串替换，不理解 C++ 语法（比如变量、函数）
2. 预处理阶段执行：早于编译、链接，替换后代码才进入编译流程
3. 无类型安全：宏不检查数据类型，可能导致隐式类型转换错误
4. 作用域：从定义处到文件结束（可通过 #undef 手动终止）
*** 示例解释 ***
``` c++ 
1. 文本替换：#define MAX_CONN 10   
2. 宏函数：#define SWAP(a, b) do { auto temp = (a); (a) = (b); (b) = temp; } while(0) 需传变量，不能传字面量
3. 条件编译：#define DEBUG
4. 字符串化（#）与连接（##）操作符：#define TO_STRING(x) #x 将参数转为字符串
5. 简化重复代码：#define SQL_TRY_BEGIN try {
                #define SQL_TRY_END } catch (const exception& e) { cerr << "SQL 错误：" << e.what() << endl; }
6. 头文件保护：属于「条件编译」的延伸
    #ifndef SINGLE_PATTERN_H  // 1. 检查宏 SINGLE_PATTERN_H 是否未定义
    #define SINGLE_PATTERN_H  // 2. 若未定义，定义该宏（标记为“已包含”）
    #endif // SINGLE_PATTERN_H  // 4. 结束条件编译
```
## c++类的一些特性
1. 继承与多态
分三类：public/private/protected
*** 虚函数 ***
eg：virtual double area()const{return 0;} 重写 double area() const override{ return 3.14*r*r}
*** 纯虚函数 ***
eg: virtual double area() const = 0;包含纯虚函数的是抽象类，只能继承不能实现
## ->和.的区别
eg：auto comp_ptr = std::make_unique<SimpleComponent>(); 
    这句话的意思是创建智能（独占）指针指向SimpleComponent这个类的实例
    ->是和指针配套使用，.是和实例配套使用
    comp_ptr->chengyuan等价于(*comp_ptr).chengyuan
## 深拷贝与浅拷贝
浅拷贝：只拷贝对象的「表层数据」（如栈上的基本类型、指针 / 引用），不拷贝指针指向的「底层资源」。拷贝后，原对象和新对象共享同一份底层资源。
深拷贝：不仅拷贝表层数据，还会为新对象「重新分配独立的底层资源」，并将原资源的内容复制到新资源中。拷贝后，原对象和新对象完全独立，互不影响。
## c++的隐式转换
隐式转换（Implicit Conversion）是指编译器在不经过程序员显式指令的情况下，自动将一种数据类型转换为另一种数据类型 的行为
## emplace关键字
核心作用是直接在容器的内存空间里构造对象
1. 与new的区别
new 是手动分配内存 + 构造对象，需要手动管理生命周期；
emplace 是利用容器的内存构造对象，生命周期由容器托管，无需手动干预。

## 析构函数和delete关键字的联系与区别
	                析构函数（~类名()）	                                delete 运算符
核心职责	    清理对象内部资源（如成员变量的堆内存、文件句柄）	          1. 调用析构函数清理内部资源；2. 释放对象本身的内存
作用对象	    任意对象（栈 / 堆 / 容器）	                               仅堆对象（new 创建的对象）
是否手动调用	绝对不要手动调用（除非是 placement new 场景）	            必须手动调用（裸 new 创建的对象），或由智能指针自动调用
内存释放范围	不释放任何内存（仅清理资源）	                            释放对象本身占用的堆内存

## c++ STL容器
序列式容器	vector、list、deque	数据按插入顺序存储，可随机 / 顺序访问	简单数据存储、频繁增删 / 访问
关联式容器	map、set、unordered_map	数据按键（key）排序 / 哈希存储，查找效率高	快速查找、去重、键值对存储
容器适配器	stack、queue、priority_queue	基于基础容器封装，提供特定操作接口（如栈的先进后出）
*** deque、list、set、priority_queue ***
1. deque:双端队列 push(pop)_back(front)
   *** 底层实现 ***：分段连续的动态数组（而非单一连续内存），通过中控器管理多个缓冲区，既兼顾了vector的随机访问效率，又解决了vector头部插入效率低的问题；
   核心特点：
            支持随机访问（[]运算符、at()），效率略低于vector；
            头部 / 尾部插入 / 删除元素效率高（O (1)）；
            中间插入 / 删除效率低（O (n)），需要移动元素；
            存分配比vector灵活，扩容时无需整体拷贝。
2. list:双向列表 存在迭代器，通过迭代器可以任意添加/删除元素，无法随机访问
   *** 底层实现 ***：双向循环链表，每个节点包含数据、前驱指针和后继指针
   核心特点：
            不支持随机访问（不能用[]或at()），只能通过迭代器顺序遍历；
            任意位置插入 / 删除元素效率高（O (1)），只需修改指针，无需移动元素；
            内存开销比vector/deque大（每个节点额外存储两个指针）；
            遍历效率低于连续内存容器（vector/deque），因为会产生内存碎片、缓存不友好。
``` c++
    list<int> lst = {1, 3, 5};
    
    // 1. 任意位置插入
    auto it = lst.begin();
    ++it; // 指向3的位置
    lst.insert(it, 2); // 在3前插入2：[1,2,3,5]
    
    // 2. 删除元素
    lst.remove(5); // 删除所有值为5的元素：[1,2,3]
    
    // 3. 排序（list自带sort，比STL通用sort更高效）
    lst.push_back(0);
    lst.sort(); // 升序排序：[0,1,2,3]
```
3. set:集合
   *** 底层实现 ***：红黑树（一种平衡二叉搜索树）insert和find以及erase set<int, greater<int>> s2 = {1,3,2};//降序排序
    核心特点：
            元素唯一：插入重复元素会被自动忽略；
            元素有序：默认按<运算符升序排列，也可自定义排序规则；
            查找效率高：O (log n)，远高于序列式容器；
            不支持随机访问，只能通过迭代器遍历；
            不能直接修改元素值（因为修改会破坏有序性，需先删除再插入）。
4. priority_queue:优先级队列
   *** 底层实现 ***：默认基于vector（也可指定deque）封装，底层数据结构是大顶堆（默认）
   核心特点：
            队列特性：只能访问队首（优先级最高）元素，不能遍历；
            优先级规则：默认按<运算符，数值大的优先级高（大顶堆），可自定义为小顶堆；
            插入 / 删除效率：O (log n)，堆的调整开销；
            常用操作：入队（push）、出队（pop）、访问队首（top）
## explicit关键字和constexpr关键字
1. explicit关键字：用于构造函数，防止隐式类型转换
    为什么需要：防止 unintended type conversions（隐式类型转换），导致错误的结果或逻辑错误。
    避免意外创建实例，造成资源消耗
2. constexpr关键字：用于函数和变量，在编译时求值，提高效率
    阶段	时机	执行主体	主要任务	示例                            此时的文件变化
    预处理	编译期	预处理器	处理头文件、宏、条件编译	#include 展开     cpp到.i文件(预处理后的源码)
    编译	编译期	编译器	    语法分析、类型检查、生成汇编	类型错误检查   .i到.s文件(汇编代码)
    汇编	编译期	汇编器	    生成机器码	.s → .o                          .s到.o文件(机器码，目标文件)
    链接	编译期	链接器	    符号解析、库链接	链接 libstdc++            .o到可执行文件(elf文件)
    运行时	运行时	CPU/操作系统	内存分配、I/O、线程管理	动态内存分配
## 模板的用法
1. 函数模板：定义通用的函数，可用于不同类型的参数。
2. 类模板：定义通用的类，可用于不同类型的成员变量。
3. 模板特化：针对特定类型提供定制的实现，优化性能或满足特殊需求。
eg:
``` c++
    template<typename T>
    T max(T a, T b) {
        return a > b ? a : b;
    }
```
*** 可变参数模板 ***
typename ... Arguments
typename...：是C++11引入的可变参数模板声明符，表明后续标识符可以接受零个或多个类型参数
eg:
``` c++
    template<typename Callable, typename... Arguments>
    auto submit(Callable&& task, Arguments&&... args) {
        // ...
    }
```
Arguments&&... args：是函数参数包，与模板参数包Arguments对应，用于接收任意数量、任意类型的函数实参；
... 用于展开参数包，将传入的多个实参分别传递给可调用对象。
&&：右值引用，用于转发参数，避免不必要的拷贝，提高效率。
完美转发：将参数原封不动地传递给另一个函数，保留其值类别（左值或右值）和const属性。
## 函数对象（仿函数）
函数对象是一种可以像函数一样被调用的对象，其核心是在类中重载了 operator() 运算符。
eg:
``` c++
    struct Add {
        template<typename T, typename U>
        auto operator()(T&& a, U&& b) const {
            return std::forward<T>(a) + std::forward<U>(b);
        }
    };
```
*** 函数对象的优势 ***
1. 可作为参数传递：可以将函数对象作为参数传递给其他函数，实现回调机制。
2. 可存储在容器中：可以将多个函数对象存储在容器（如vector）中，方便统一管理。
3. 可定制行为：可以根据需要定制函数对象的行为，实现灵活的功能。
## bind的用法
std::bind(可调用对象, 参数1, 参数2, ...)
1. 绑定参数：可以将函数的参数绑定到特定的值，生成一个新的可调用对象。
   
2. 延迟调用：可以将函数调用延迟到稍后执行，实现异步或事件驱动的编程。

3. 部分应用：可以将函数的部分参数绑定，生成一个新的函数，方便复用。
   eg:
``` c++
    auto add5 = std::bind(Add(), std::placeholders::_1, 5);
    int result = add5(3); // 8
```
4. 绑定成员函数：可以将类的成员函数绑定到特定对象，生成一个新的可调用对象。
   eg:
``` c++
    class MyClass {
    public:
        void print(int x) { std::cout << x << std::endl; }
    };
    MyClass obj;
    auto printObj = std::bind(&MyClass::print, &obj, std::placeholders::_1);
    printObj(42); // 输出：42
```
与函数包装器的区别：
1. 功能：函数包装器（如std::function）用于存储和调用可调用对象，而bind用于创建新的可调用对象。
2. 绑定参数：函数包装器只能绑定固定参数，而bind可以绑定部分参数，生成新的函数。
## 函数包装器（本质是重载了（）的模板类）
和仿函数的关系：
几种可以封装的对象：
1. 函数指针
eg:
``` c++
    int add(int a, int b) { return a + b; }
    std::function<int(int, int)> func = add;
```
2. 函数对象（仿函数）
eg:
``` c++
    struct Add {
        template<typename T, typename U>
        auto operator()(T&& a, U&& b) const {
            return std::forward<T>(a) + std::forward<U>(b);
        }
    };
    std::function<int(int, int)> func = Add();
```
3. Lambda表达式
eg:
``` c++
    auto add = [](int a, int b) { return a + b; };
    std::function<int(int, int)> func = add;
```
## invoke_result_t
用于获取函数调用的返回类型，在编译期确定。
eg:
``` c++
    std::invoke_result_t<Add, int, int> result; // result 类型为 int
    第一个是函数类型，后面是参数列表
```
## 可调用对象
在C++中，可调用对象（Callable Object）是一个核心概念，指的是能够通过函数调用运算符 () 进行调用的实体。简单来说，任何可以像函数一样使用 obj(...) 语法调用的东西，都可以称为可调用对象
*** invoke和function的区别 ***
特性	std::invoke	std::function
核心功能	统一调用	统一存储
类型擦除	无（编译期）	有（运行时）
性能开销	几乎为0（编译期展开）	较小（虚函数调用）
使用场景	泛型编程中的统一调用	回调函数、事件处理
结果	返回调用结果	存储可调用对象
eg:
``` c++
 // 调用 Lambda 表达式
    auto lambda = [](int a, int b) { return a - b; };
    int result3 = std::invoke(lambda, 5, 2);  // 等价于 lambda(5, 2)
```
## 完美转发
&和&&(左右值引用)
forward<T>(arg)
## range库
用于在字符串容器中查找长度最大的字符串
eg:
``` c++
    std::string str = "hello world";
    auto max_len = std::max_element(str.begin(), str.end(),
        [](const std::string& a, const std::string& b) {
            return a.size() < b.size();
        });
```