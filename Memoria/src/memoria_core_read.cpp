#include "memoria_core_read.hpp"

#include "memoria_core_options.hpp"
#include "memoria_core_misc.hpp"
#include "memoria_core_errors.hpp"

#include "memoria_utils_string.hpp"

MEMORIA_BEGIN

std::optional<uint8_t> ReadU8(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}

	return *reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(addr) + offset);
}

std::optional<uint16_t> ReadU16(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}
	return *reinterpret_cast<uint16_t *>(reinterpret_cast<uintptr_t>(addr) + offset);
}

std::optional<uint32_t> ReadU24(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}

	uint32_t value = (*reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(addr) + offset)) |
		((*reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(addr) + offset + 1)) << 8) |
		((*reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(addr) + offset + 2)) << 16);

	return value;
}

std::optional<uint32_t> ReadU32(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}
	return *reinterpret_cast<uint32_t *>(reinterpret_cast<uintptr_t>(addr) + offset);
}

std::optional<uint64_t> ReadU64(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}

	return *reinterpret_cast<uint64_t *>(reinterpret_cast<uintptr_t>(addr) + offset);
}

std::optional<int8_t> ReadI8(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}

	return *reinterpret_cast<int8_t *>(reinterpret_cast<uintptr_t>(addr) + offset);
}

std::optional<int16_t> ReadI16(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}

	return *reinterpret_cast<int16_t *>(reinterpret_cast<uintptr_t>(addr) + offset);
}

std::optional<int32_t> ReadI24(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}

	uint32_t value = (*reinterpret_cast<const uint8_t *>(reinterpret_cast<uintptr_t>(addr) + offset)) |
		((*reinterpret_cast<const uint8_t *>(reinterpret_cast<uintptr_t>(addr) + offset + 1)) << 8) |
		((*reinterpret_cast<const uint8_t *>(reinterpret_cast<uintptr_t>(addr) + offset + 2)) << 16);

	if (value & 0x800000)
		value |= 0xFF000000;

	return static_cast<int32_t>(value);
}

std::optional<int32_t> ReadI32(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}

	return *reinterpret_cast<int32_t *>(reinterpret_cast<uintptr_t>(addr) + offset);
}

std::optional<int64_t> ReadI64(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}

	return *reinterpret_cast<int64_t *>(reinterpret_cast<uintptr_t>(addr) + offset);
}

std::optional<float> ReadFloat(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}

	return *reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(addr) + offset);
}

std::optional<double> ReadDouble(const void *addr, ptrdiff_t offset)
{
	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return std::nullopt;
	}

	return *reinterpret_cast<double *>(reinterpret_cast<uintptr_t>(addr) + offset);
}

bool ReadAStr(const void *addr, char *out, size_t max_size, ptrdiff_t offset)
{
	if (!out || max_size == 0)
		return false;

	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return false;
	}

	const char *src = reinterpret_cast<const char *>(reinterpret_cast<uintptr_t>(addr) + offset);
	StrNCopySafeA(out, max_size, src, _TRUNCATE);
	return true;
}

bool ReadWStr(const void *addr, wchar_t *out, size_t max_size, ptrdiff_t offset)
{
	if (!out || max_size == 0)
		return false;

	if (IsSafeModeActive() && !IsMemoryValid(addr, offset))
	{
		SetError(ME_INVALID_MEMORY);
		return false;
	}

	const wchar_t *src = reinterpret_cast<const wchar_t *>(reinterpret_cast<uintptr_t>(addr) + offset);
	StrNCopySafeW(out, max_size, src, _TRUNCATE);
	return true;
}

bool GetMemoryBlock(const void *source, size_t size, void *dest)
{
	if (source == nullptr || dest == nullptr)
		return false;

	MemCopy(dest, source, size);
	return true;
}

std::span<const uint8_t> GetMemorySpan(const void *source, size_t size)
{
	return std::span<const uint8_t>(static_cast<const uint8_t *>(source), size);
}

std::span<uint8_t> GetMemorySpan(void *source, size_t size)
{
	return std::span<uint8_t>(static_cast<uint8_t *>(source), size);
}

Memoria::Vector<uint8_t> GetMemoryData(const void *source, size_t size)
{
	Memoria::Vector<uint8_t> data(size);

	if (source != nullptr)
		MemCopy(data.data(), source, size);

	return data;
}

MEMORIA_END