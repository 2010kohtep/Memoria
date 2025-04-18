#include "memoria_core_windows.hpp"

#ifndef MEMORIA_DISABLE_CORE_SIGNATURE

#include "memoria_core_misc.hpp"
#include "memoria_utils_string.hpp"

#include <VersionHelpers.h>
#include <string_view>

MEMORIA_BEGIN

using SectionCallbackFn = bool(*)(PIMAGE_SECTION_HEADER section, LPVOID param);

void EnumSections(HMODULE handle, SectionCallbackFn cb, LPVOID param)
{
	if (!cb)
		return;

	if (!handle)
		handle = GetModuleHandleA(0);

	if (!handle)
		return;

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)handle;
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)dosHeader + dosHeader->e_lfanew);

	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);

	for (unsigned int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
	{
		if (!cb(section, param))
			break;

		section++;
	}
}

PIMAGE_SECTION_HEADER GetSectionByIndex(HMODULE handle, DWORD image_directory)
{
	if (!handle)
		handle = GetModuleHandleA(0);

	if (!handle)
		return nullptr;

	if (image_directory >= IMAGE_NUMBEROF_DIRECTORY_ENTRIES)
		return nullptr;

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)handle;
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)dosHeader + dosHeader->e_lfanew);

	auto dataDir = ntHeaders->OptionalHeader.DataDirectory[image_directory];

	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);

	for (unsigned int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
	{
		if (dataDir.VirtualAddress == section->VirtualAddress)
			return section;

		section++;
	}

	return nullptr;
}

PIMAGE_SECTION_HEADER GetSectionByFlags(HMODULE handle, DWORD flags, bool match_exactly)
{
	if (!handle)
		handle = GetModuleHandleA(0);

	if (!handle)
		return nullptr;

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)handle;
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)dosHeader + dosHeader->e_lfanew);

	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);

	for (unsigned int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
	{
		if (match_exactly)
		{
			if (section->Characteristics == flags)
			{
				return section;
			}
		}
		else
		{
			if ((section->Characteristics & flags) != 0)
			{
				return section;
			}
		}

		section++;
	}

	return nullptr;
}

PIMAGE_SECTION_HEADER GetSectionByName(HMODULE handle, const char *name)
{
	if (!handle)
		handle = GetModuleHandleA(nullptr);

	if (!handle || !name || !*name)
		return nullptr;

	PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);
	PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
		reinterpret_cast<DWORD_PTR>(dosHeader) + dosHeader->e_lfanew);

	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);

	size_t name_len = StrLenA(name);

	for (unsigned int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
	{
		if (CompareMemory(section->Name, name, name_len) == 0)
		{
			return section;
		}

		section++;
	}

	return nullptr;
}

PIMAGE_SECTION_HEADER GetEntrySection(HMODULE handle)
{
	if (!handle)
		handle = GetModuleHandleA(0);

	if (!handle)
		return nullptr;

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)handle;
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)dosHeader + dosHeader->e_lfanew);

	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);
	LPVOID pEntryPoint = PtrOffset(handle, ntHeaders->OptionalHeader.AddressOfEntryPoint);

	for (unsigned int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
	{
		auto &&[lo, hi] = GetSectionBounds(section);

		if (IsInBounds(pEntryPoint, lo, hi))
			return section;

		section++;
	}

	return nullptr;
}

std::pair<PVOID, PVOID> GetSectionBounds(HMODULE handle, PIMAGE_SECTION_HEADER section)
{
	if (!handle)
		handle = GetModuleHandleA(0);

	if (!handle)
		return {};

	auto sectionStart = (PVOID)((DWORD_PTR)handle + section->VirtualAddress);
	auto sectionEnd = (PVOID)((DWORD_PTR)sectionStart + section->Misc.VirtualSize - 1);

	return std::make_pair(sectionStart, sectionEnd);
}

std::pair<PVOID, PVOID> GetSectionBounds(PIMAGE_SECTION_HEADER section)
{
	HMODULE handle;

	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)section, &handle))
		return {};

	if (!handle)
		return {};

	return GetSectionBounds(handle, section);
}

void *GetImageBase(HMODULE handle)
{
	if (!handle)
		handle = GetModuleHandleA(0);

	if (!handle)
		return nullptr;

	IMAGE_DOS_HEADER *pDOSHeader = (IMAGE_DOS_HEADER *)handle;

	if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return nullptr;

	IMAGE_NT_HEADERS *pNTHeaders = (IMAGE_NT_HEADERS *)((BYTE *)pDOSHeader + pDOSHeader->e_lfanew);

	if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE)
		return nullptr;

	return (void *)pNTHeaders->OptionalHeader.ImageBase;
}

void *GetImageBase(const char *name)
{
	HMODULE handle;

	if (!name || !*name)
		handle = GetModuleHandleA(nullptr);
	else
		handle = GetModuleHandleA(name);

	if (!handle)
		return nullptr;

	return GetImageBase(handle);
}

std::pair<WORD, WORD> GetModuleVersion(HMODULE handle)
{
	if (!handle)
		handle = GetModuleHandleA(0);

	if (!handle)
		return { 0, 0 };

	IMAGE_DOS_HEADER *pDOSHeader = (IMAGE_DOS_HEADER *)handle;
	if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return { 0, 0 };

	IMAGE_NT_HEADERS *pNTHeaders = (IMAGE_NT_HEADERS *)((BYTE *)pDOSHeader + pDOSHeader->e_lfanew);
	if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE)
		return { 0, 0 };

	return
	{
		pNTHeaders->OptionalHeader.MajorOperatingSystemVersion,
		pNTHeaders->OptionalHeader.MinorOperatingSystemVersion
	};
}

std::pair<WORD, WORD> GetModuleVersion(const char *name)
{
	HMODULE handle;

	if (!name || !*name)
		handle = GetModuleHandleA(nullptr);
	else
		handle = GetModuleHandleA(name);

	if (!handle)
		return { 0, 0 };

	return GetModuleVersion(handle);
}

DWORD GetModuleSize(HMODULE handle)
{
	if (!handle)
		handle = GetModuleHandleA(0);

	PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);
	PIMAGE_NT_HEADERS nt = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(dos) + dos->e_lfanew);

	return nt->OptionalHeader.SizeOfImage;
}

MEMORIA_END

#endif