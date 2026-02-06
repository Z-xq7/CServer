#pragma once
#include<memory>
#include<mutex>
#include<iostream>

//using namespace std;

//只有实例化的时候才会创建实例，保证了单例模式的特点，同时使用智能指针管理内存，避免了内存泄漏的问题。
template<typename T>
class Singleton
{
protected:
	Singleton() = default;
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator=(const Singleton<T>& st) = delete;

	static std::shared_ptr<T> _instance;

public:
	~Singleton()
	{
		std::cout << "Singleton destructed" << std::endl;
	}

	static std::shared_ptr<T> GetInstance()
	{
		static std::once_flag s_flag;	//只有第一次调用时才会初始化实例，保证线程安全
		std::call_once(s_flag, [&]()	//call_once 接受一个函数对象作为参数，并确保该函数对象只被调用一次
		{
			_instance = std::shared_ptr<T>(new T());	//使用 shared_ptr 管理内存
			//为什么不使用 make_shared：因为std::make_shared<T>() 构造私有构造函数的类型。
			//LogicSystem 将构造函数设为 private，并仅将 Singleton<LogicSystem> 设为友元，
			//但 std::make_shared<T> 在标准库内部调用构造函数，并不属于你的友元类，因此访问被拒绝
		});

		return _instance;
	}

	void PrintAddress()
	{
		std::cout << "Singleton instance address: " << this << std::endl;
	}
};

template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;	//静态成员变量需要在类外定义和初始化