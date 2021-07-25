#include "Utils.hpp"

#define NOMINMAX
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <WinInet.h>

#include "../valve_sdk/csgostructs.hpp"
#include "Math.hpp"
#include <map>
#include "../options.hpp"
#include <Psapi.h>
HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in = NULL, _old_in = NULL;
std::map<const char*, HMODULE> modules;

namespace Utils {
	std::vector<char> HexToBytes(const std::string& hex) {
		std::vector<char> res;

		for (auto i = 0u; i < hex.length(); i += 2) {
			std::string byteString = hex.substr(i, 2);
			char byte = (char)strtol(byteString.c_str(), NULL, 16);
			res.push_back(byte);
		}

		return res;
	}
	std::string BytesToString(unsigned char* data, int len) {
		constexpr char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
		std::string res(len * 2, ' ');
		for (int i = 0; i < len; ++i) {
			res[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
			res[2 * i + 1] = hexmap[data[i] & 0x0F];
		}
		return res;
	}
	std::vector<std::string> Split(const std::string& str, const char* delim) {
		std::vector<std::string> res;
		char* pTempStr = _strdup(str.c_str());
		char* context = NULL;
		char* pWord = strtok_s(pTempStr, delim, &context);
		while (pWord != NULL) {
			res.push_back(pWord);
			pWord = strtok_s(NULL, delim, &context);
		}

		free(pTempStr);

		return res;
	}

	unsigned int FindInDataMap(datamap_t *pMap, const char *name) {
		while (pMap) {
			for (int i = 0; i<pMap->dataNumFields; i++) {
				if (pMap->dataDesc[i].fieldName == NULL)
					continue;

				if (strcmp(name, pMap->dataDesc[i].fieldName) == 0)
					return pMap->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

				if (pMap->dataDesc[i].fieldType == FIELD_EMBEDDED) {
					if (pMap->dataDesc[i].td) {
						unsigned int offset;

						if ((offset = FindInDataMap(pMap->dataDesc[i].td, name)) != 0)
							return offset;
					}
				}
			}
			pMap = pMap->baseMap;
		}

		return 0;
	}
    /*
     * @brief Create console
     *
     * Create and attach a console window to the current process
     */
    void AttachConsole()
    {
        _old_out = GetStdHandle(STD_OUTPUT_HANDLE);
        _old_err = GetStdHandle(STD_ERROR_HANDLE);
        _old_in  = GetStdHandle(STD_INPUT_HANDLE);

        ::AllocConsole() && ::AttachConsole(GetCurrentProcessId());

        _out     = GetStdHandle(STD_OUTPUT_HANDLE);
        _err     = GetStdHandle(STD_ERROR_HANDLE);
        _in      = GetStdHandle(STD_INPUT_HANDLE);

        SetConsoleMode(_out,
            ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

        SetConsoleMode(_in,
            ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
            ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
    }

    /*
     * @brief Detach console
     *
     * Detach and destroy the attached console
     */
    void DetachConsole()
    {
        if(_out && _err && _in) {
            FreeConsole();

            if(_old_out)
                SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
            if(_old_err)
                SetStdHandle(STD_ERROR_HANDLE, _old_err);
            if(_old_in)
                SetStdHandle(STD_INPUT_HANDLE, _old_in);
        }
    }

    /*
     * @brief Print to console
     *
     * Replacement to printf that works with the newly created console
     */
    bool ConsolePrint(const char* fmt, ...)
    {
        if(!_out) 
            return false;

        char buf[1024];
        va_list va;

        va_start(va, fmt);
        _vsnprintf_s(buf, 1024, fmt, va);
        va_end(va);

        return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
    }

    /*
     * @brief Blocks execution until a key is pressed on the console window
     *
     */
    char ConsoleReadKey()
    {
        if(!_in)
            return false;

        auto key = char{ 0 };
        auto keysread = DWORD{ 0 };

        ReadConsoleA(_in, &key, 1, &keysread, nullptr);

        return key;
    }


    /*
     * @brief Wait for all the given modules to be loaded
     *
     * @param timeout How long to wait
     * @param modules List of modules to wait for
     *
     * @returns See WaitForSingleObject return values.
     */
    int WaitForModules(std::int32_t timeout, const std::initializer_list<std::wstring>& modules)
    {
        bool signaled[32] = { 0 };
        bool success = false;

        std::uint32_t totalSlept = 0;

        if(timeout == 0) {
            for(auto& mod : modules) {
                if(GetModuleHandleW(std::data(mod)) == NULL)
                    return WAIT_TIMEOUT;
            }
            return WAIT_OBJECT_0;
        }

        if(timeout < 0)
            timeout = INT32_MAX;

        while(true) {
            for(auto i = 0u; i < modules.size(); ++i) {
                auto& module = *(modules.begin() + i);
                if(!signaled[i] && GetModuleHandleW(std::data(module)) != NULL) {
                    signaled[i] = true;

                    //
                    // Checks if all modules are signaled
                    //
                    bool done = true;
                    for(auto j = 0u; j < modules.size(); ++j) {
                        if(!signaled[j]) {
                            done = false;
                            break;
                        }
                    }
                    if(done) {
                        success = true;
                        goto exit;
                    }
                }
            }
            if(totalSlept > std::uint32_t(timeout)) {
                break;
            }
            Sleep(10);
            totalSlept += 10;
        }

    exit:
        return success ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
    }

    /*
     * @brief Scan for a given byte pattern on a module
     *
     * @param module    Base of the module to search
     * @param signature IDA-style byte array pattern
     *
     * @returns Address of the first occurence
     */
    HMODULE GetModule(const char* name)
    {
        if (!modules.count(name) || !modules[name])
            modules[name] = GetModuleHandleA(name);

        return modules[name];
    }
    std::uint8_t* PatternScan2(const char* module_name, const char* signature)
    {
        auto module = GetModule(module_name);

        static auto pattern_to_byte = [](const char* pattern) {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for (auto current = start; current < end; ++current) {
                if (*current == '?') {
                    ++current;
                    if (*current == '?')
                        ++current;
                    bytes.push_back(-1);
                }
                else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)module;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = pattern_to_byte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for (auto i = 0ul; i < sizeOfImage - s; ++i) {
            bool found = true;
            for (auto j = 0ul; j < s; ++j) {
                if (scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return &scanBytes[i];
            }
        }
        return nullptr;
    }

    static std::pair<void*, std::size_t> GetModuleInformation(const char* name) noexcept
    {
#ifdef _WIN32
        if (HMODULE handle = GetModuleHandleA(name)) {
            if (MODULEINFO moduleInfo; GetModuleInformation(GetCurrentProcess(), handle, &moduleInfo, sizeof(moduleInfo)))
                return std::make_pair(moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage);
        }
        return {};
#elif __linux__
        struct ModuleInfo {
            const char* name;
            void* base = nullptr;
            std::size_t size = 0;
        } moduleInfo;

        moduleInfo.name = name;

        dl_iterate_phdr([](struct dl_phdr_info* info, std::size_t, void* data) {
            const auto moduleInfo = reinterpret_cast<ModuleInfo*>(data);
            if (!std::string_view{ info->dlpi_name }.ends_with(moduleInfo->name))
                return 0;

            if (const auto fd = open(info->dlpi_name, O_RDONLY); fd >= 0) {
                if (struct stat st; fstat(fd, &st) == 0) {
                    if (const auto map = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0); map != MAP_FAILED) {
                        const auto ehdr = (ElfW(Ehdr)*)map;
                        const auto shdrs = (ElfW(Shdr)*)(std::uintptr_t(ehdr) + ehdr->e_shoff);
                        const auto strTab = (const char*)(std::uintptr_t(ehdr) + shdrs[ehdr->e_shstrndx].sh_offset);

                        for (auto i = 0; i < ehdr->e_shnum; ++i) {
                            const auto shdr = (ElfW(Shdr)*)(std::uintptr_t(shdrs) + i * ehdr->e_shentsize);

                            if (std::strcmp(strTab + shdr->sh_name, ".text") != 0)
                                continue;

                            moduleInfo->base = (void*)(info->dlpi_addr + shdr->sh_offset);
                            moduleInfo->size = shdr->sh_size;
                            munmap(map, st.st_size);
                            close(fd);
                            return 1;
                        }
                        munmap(map, st.st_size);
                    }
                }
                close(fd);
            }

            moduleInfo->base = (void*)(info->dlpi_addr + info->dlpi_phdr[0].p_vaddr);
            moduleInfo->size = info->dlpi_phdr[0].p_memsz;
            return 1;
            }, &moduleInfo);

        return std::make_pair(moduleInfo.base, moduleInfo.size);
#endif
    }

    std::vector<int> PatternToBytes(const char* szPattern)
    {
        std::vector<int> vecBytes = { };
        char* chStart = const_cast<char*>(szPattern);
        char* chEnd = chStart + strlen(szPattern);

        // convert pattern into bytes
        for (char* chCurrent = chStart; chCurrent < chEnd; ++chCurrent)
        {
            // check is current byte a wildcard
            if (*chCurrent == '?')
            {
                ++chCurrent;

                // check is next byte is also wildcard
                if (*chCurrent == '?')
                    ++chCurrent;

                // ignore that
                vecBytes.push_back(-1);
            }
            else
                // convert byte to hex
                vecBytes.push_back(strtoul(chCurrent, &chCurrent, 16));
        }

        return vecBytes;
    }

    [[nodiscard]] static auto GenerateBadCharTable(std::string_view pattern) noexcept
    {
        assert(!pattern.empty());

        std::array<std::size_t, (std::numeric_limits<std::uint8_t>::max)() + 1> table;

        auto lastWildcard = pattern.rfind('?');
        if (lastWildcard == std::string_view::npos)
            lastWildcard = 0;

        const auto defaultShift = (std::max)(std::size_t(1), pattern.length() - 1 - lastWildcard);
        table.fill(defaultShift);

        for (auto i = lastWildcard; i < pattern.length() - 1; ++i)
            table[static_cast<std::uint8_t>(pattern[i])] = pattern.length() - 1 - i;

        return table;
    }

    std::uintptr_t FindPattern(const char* moduleName, std::string_view pattern)
    {
        static auto id = 0;
        ++id;

        const auto [moduleBase, moduleSize] = GetModuleInformation(moduleName);

        if (moduleBase && moduleSize) {
            const auto lastIdx = pattern.length() - 1;
            const auto badCharTable = GenerateBadCharTable(pattern);

            auto start = static_cast<const char*>(moduleBase);
            const auto end = start + moduleSize - pattern.length();

            while (start <= end) {
                int i = lastIdx;
                while (i >= 0 && (pattern[i] == '?' || start[i] == pattern[i]))
                    --i;

                if (i < 0)
                    return reinterpret_cast<std::uintptr_t>(start);

                start += badCharTable[static_cast<std::uint8_t>(start[lastIdx])];
            }
        }
#ifdef _WIN32
        MessageBoxA(NULL, ("Failed to find pattern #" + std::to_string(id) + '!').c_str(), "Osiris", MB_OK | MB_ICONWARNING);
#endif
        return 0;
    }

    std::uintptr_t FindPattern(std::uint8_t* uRegionStart, std::uintptr_t uRegionSize, const char* szPattern)
    {
        std::vector<int> vecBytes = PatternToBytes(szPattern);

        // check for bytes sequence match
        for (unsigned long i = 0UL; i < uRegionSize - vecBytes.size(); ++i)
        {
            bool bByteFound = true;

            for (unsigned long s = 0UL; s < vecBytes.size(); ++s)
            {
                // check if doesn't match or byte isn't a wildcard
                if (uRegionStart[i + s] != vecBytes[s] && vecBytes[s] != -1)
                {
                    bByteFound = false;
                    break;
                }
            }

            // return valid address
            if (bByteFound)
                return reinterpret_cast<std::uintptr_t>(&uRegionStart[i]);
        }

        return 0U;
    }

    std::uintptr_t FindPattern2(const char* szModuleName, const char* szPattern)
    {
        const HMODULE hModule = GetModuleHandleA(szModuleName);

        if (!hModule)
            throw std::runtime_error("FindPattern failed");

        const auto uModuleAdress = reinterpret_cast<std::uint8_t*>(hModule);
        const auto pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
        const auto pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(uModuleAdress + pDosHeader->e_lfanew);
        const std::uintptr_t uOffset = FindPattern(uModuleAdress, pNtHeaders->OptionalHeader.SizeOfImage, szPattern);

        return uOffset;
    }



    std::uint8_t* PatternScan(void* module, const char* signature)
    {
        static auto pattern_to_byte = [](const char* pattern) {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for(auto current = start; current < end; ++current) {
                if(*current == '?') {
                    ++current;
                    if(*current == '?')
                        ++current;
                    bytes.push_back(-1);
                } else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)module;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = pattern_to_byte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for(auto i = 0ul; i < sizeOfImage - s; ++i) {
            bool found = true;
            for(auto j = 0ul; j < s; ++j) {
                if(scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }
            if(found) {
                return &scanBytes[i];
            }
        }
        return nullptr;
    }

    /*
     * @brief Set player clantag
     *
     * @param tag New clantag
     */
    void SetClantag(const char* tag)
    {
        static auto fnClantagChanged = (int(__fastcall*)(const char*, const char*))PatternScan(GetModuleHandleW(L"engine.dll"), "53 56 57 8B DA 8B F9 FF 15");

        fnClantagChanged(tag, tag);
    }

    void Clantag() {
        /* Credits: https://www.unknowncheats.me/forum/counterstrike-global-offensive/392266-semi-sync-clantag.html */
        static bool isReset = true;

        if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected() || !g_Options.misc_clantag || (g_Options.misc_clantag_type == 1 && !strlen(g_Options.misc_clantag_string)))
        {
            if (!isReset)
            {
                SetClantag("");
                isReset = true;
            }

            return;
        }

        static std::string tag = "cyberware ";
        static const size_t size = tag.length();
        static int lastTime = 0;
        static int lastValue = 0;

        int currentTime = static_cast<int>(g_GlobalVars->curtime * 2.4f);
        // Check if current time is different to the last saved time
        if (currentTime != lastTime)
        {
            // Take index in relation to time 
            int index = currentTime % (size + 1);
            // Get offset and prevent crash with % size
            int offset = (index - lastValue) % size;
            // Rotate
            std::rotate(tag.rbegin(), tag.rbegin() + offset, tag.rend());
            // Set clan tag
            g_Options.misc_clantag_type == 1 ? SetClantag(g_Options.misc_clantag_string) : Utils::SetClantag(tag.c_str());
            // Save last current time
            lastTime = currentTime;
            // Save last value, if it's last then = 0 to don't get negative value afterwards
            lastValue = index != size ? index : 0;
        }

        if (isReset)
            isReset = false;
    }

    /*
     * @brief Set player name
     *
     * @param name New name
     */
    void SetName(const char* name)
    {
        static auto nameConvar = g_CVar->FindVar("name");
        nameConvar->m_fnChangeCallbacks.m_Size = 0;

        // Fix so we can change names how many times we want
        // This code will only run once because of `static`
        static auto do_once = (nameConvar->SetValue("\n���"), true);

        nameConvar->SetValue(name);
    }

    int GetFPS()
    {
        if (!g_GlobalVars) return 0;

        static float m_Framerate = 0;
        m_Framerate = 0.9 * m_Framerate + (1.0 - 0.9) * g_GlobalVars->absoluteframetime; // https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/client/vgui_netgraphpanel.cpp#L744

        return int(1.f / m_Framerate);
    };

    bool IsDangerZone()
    {
        return g_GameTypes->GetCurrentGameType() == 6;
    }

    float AngleDiff(float destAngle, float srcAngle) {
        float delta;

        delta = fmodf(destAngle - srcAngle, 360.0f);
        if (destAngle > srcAngle) if (delta >= 180) delta -= 360;
        else if (delta <= -180) delta += 360;

        return delta;
    }
}

