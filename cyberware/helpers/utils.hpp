#pragma once

#define NOMINMAX
#include <Windows.h>
#include <string>
#include <initializer_list>
#include "../valve_sdk/sdk.hpp"

namespace Utils {
	std::vector<char> HexToBytes(const std::string& hex);
	std::string BytesToString(unsigned char* data, int len);
	std::vector<std::string> Split(const std::string& str, const char* delim);
	unsigned int FindInDataMap(datamap_t * pMap, const char * name);
    /*
     * @brief Create console
     *
     * Create and attach a console window to the current process
     */
	void AttachConsole();

    /*
     * @brief Detach console
     *
     * Detach and destroy the attached console
     */
    void DetachConsole();

    /*
     * @brief Print to console
     *
     * Replacement to printf that works with the newly created console
     */
    bool ConsolePrint(const char* fmt, ...);
    
    /*
     * @brief Blocks execution until a key is pressed on the console window
     *
     */
    char ConsoleReadKey();

    /*
     * @brief Wait for all the given modules to be loaded
     *
     * @param timeout How long to wait
     * @param modules List of modules to wait for
     *
     * @returns See WaitForSingleObject return values.
     */
    int WaitForModules(std::int32_t timeout, const std::initializer_list<std::wstring>& modules);

    /*
     * @brief Scan for a given byte pattern on a module
     *
     * @param module    Base of the module to search
     * @param signature IDA-style byte array pattern
     *
     * @returns Address of the first occurence
     */
    std::uintptr_t FindPattern(const char* moduleName, std::string_view pattern);
    std::uint8_t* PatternScan(void* module, const char* signature);
    std::uintptr_t FindPattern2(const char* szModuleName, const char* szPattern);
    std::uint8_t* PatternScan2(const char* module_name, const char* signature);
    std::uintptr_t FindPattern(std::uint8_t* uRegionStart, std::uintptr_t uRegionSize, const char* szPattern);

    /*
     * @brief Set player clantag
     *
     * @param tag New clantag
     */
    void SetClantag(const char* tag);
    void Clantag();

    /*
     * @brief Set player name
     *
     * @param name New name
     */
    void SetName(const char* name);

    int GetFPS();

    bool IsDangerZone();

    float AngleDiff(float destAngle, float srcAngle);

    inline CEventListener EventListener;
}
