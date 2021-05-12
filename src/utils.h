
#pragma once
#include <windows.h>
#include <memory>
#include <tchar.h>
#include <sstream>
#include <iostream>
#include <sstream>
#include <optional>//RUST Team

#ifdef UNICODE
#define tcout std::wcout
#define tcerr std::wcerr
#define tcin std::wcin
#define tstring std::wstring
#define tstringstream std::wstringstream
#else
#define tcout std::cout
#define tcerr std::cerr
#define tcin std::cin
#define tstring std::string
#define tstringstream std::stringstream
#endif

// shortener for the TEXT macro
#define t(quote) TEXT(quote)
#define MUnique t("Controlador_42")

namespace Wrappers
{
	//only usable with HKEY and HANDLE
	template <typename H>
	class Handle;

	template <>
	class Handle<HKEY>
	{
		HKEY h{};

	public:
		Handle() {}

		~Handle()
		{
			RegCloseKey(h);
			//_tprintf(TEXT("Closed Key"));
		}
		HKEY &operator()()
		{
			return h;
		}

		HKEY copy()
		{
			return h;
		}
	};

	template <>
	class Handle<HANDLE>
	{
		HANDLE h;

	public:
		Handle(HANDLE h) : h(h) {}
		// Handle() {}

		~Handle()
		{
#ifdef _DEBUG
			tcerr << t("Closing the handle") << h << std::endl;
#endif
			CloseHandle(h);
			//_tprintf(TEXT("Closed handle"));
		}
		HANDLE copy()
		{
			return h;
		}
		HANDLE &operator()()
		{
			return h;
		}
	};
	template <>
	class Handle<HMODULE>
	{
		HMODULE h{};

	public:
		Handle() {}
		/*this overload = (HMODULE s){
			h = s;
		}*/
		~Handle()
		{
#ifdef _DEBUG
			tcerr << t("Closing the handle") << h << std::endl;
#endif
			FreeLibrary(h);
			//_tprintf(TEXT("Closed handle"));
		}
		HMODULE copy()
		{
			return h;
		}
		HMODULE &operator()()
		{
			return h;
		}
	};
};

// corre isto no inicio do programa
void init_tsets()
{
//inputs e outputs em unicode se preciso
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
}

