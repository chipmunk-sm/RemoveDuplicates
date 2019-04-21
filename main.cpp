/* Copyright (C) 2019 chipmunk-sm <dannico@linuxmail.org> */

#include <iostream>
#include <vector>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

[[noreturn]] void Usage(const char * name)
{
    std::cout << "chipmunk-sm 2019 (c) https://github.com/chipmunk-sm/" << std::endl;
    std::cout << "Remove duplicate string [and sort -s]: " << std::endl;
    std::cout << name << " [-s] \"Path and filename\"" << std::endl;
    exit(EXIT_FAILURE);
}

inline std::string trimWhitespace(std::string value)
{
    auto startPos = value.find_first_not_of(' ');

    if (startPos == std::string::npos) // no one nonspace char
        return "";

    auto endPos = value.find_last_not_of(' ') ;

    return value.substr(startPos, endPos - startPos + 1);

}

int main(int argc, char *argv[])
{

    auto bSort = false;
    std::string path;

    for (auto index = 1; index < argc; index++)
    {
        std::string tmp(argv[index]);
        if (tmp == "-s" || tmp == "-S")
            bSort = true;
        else
            path = tmp;
    }

    if(path.empty())
        Usage(argv[0]);

#if defined(_WIN32) || defined(_WIN64)
    FILE *processedFile = nullptr;
    if (fopen_s(&processedFile, path.c_str(), "r+") != 0)
        processedFile = nullptr;
#else
    auto processedFile = std::fopen(path.c_str(), "r+");
#endif
    if (processedFile == nullptr)
    {
        std::cout << "Unable to open \"" << path.c_str() << "\"" << std::endl;
        Usage(argv[0]);
    }

    std::vector<char> buffer;
    buffer.resize(65565 * 4, 0);
    std::vector<std::string> map;

    auto skip = 0;
    auto lines = 0;
    while (std::fgets(buffer.data(), static_cast<int>(buffer.size() - 10), processedFile) != nullptr)
    {
        auto tmpItem = trimWhitespace(buffer.data());
        if (tmpItem.length() < 1 || std::find(map.begin(), map.end(), tmpItem) != map.end())
        {
            skip++;
            continue;
        }
        lines++;
        map.push_back(tmpItem);
    }

    std::cout << "Source size " << std::ftell(processedFile) << " bytes" << std::endl;
    std::rewind(processedFile);

#if defined(_WIN32) || defined(_WIN64)
    auto truncateRes = _chsize_s(_fileno(processedFile), 0);
#else
    auto truncateRes = ftruncate(fileno(processedFile), 0);
#endif
    if (truncateRes != 0)
    {
        std::cout << " Failed truncate file size " << std::endl;
        exit(EXIT_FAILURE);
    }

    if (bSort)
        std::sort(map.begin(), map.end());

    for (const auto &item : map)
    {
        if(item.length())
            std::fputs(item.data(), processedFile);
    }

    std::cout << "New size " << std::ftell(processedFile) << " bytes. " << lines << " lines left. " << skip << " lines removed." << std::endl;

    if (processedFile)
        fclose(processedFile);

    return EXIT_SUCCESS;
}
