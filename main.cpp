#include <windows.h>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm> // For std::find_if
#include <iostream>

// Function prototypes
void GetCppFilesInDirectory(const std::string& directory, std::vector<std::string>& cppFiles);
void ExtractIncludesFromCppFile(const std::string& cppFileName, std::vector<std::string>& includes);
void UpdateBatchFile(const std::string& batchFileName, const std::vector<std::string>& cppFiles, const std::map<std::string, std::string>& headerFlags);

// Header to flags mapping
const std::map<std::string, std::string> headerFlags = {
    {"windows.h", "-lgdi32"},
    {"wingdi.h", "-lgdi32"},
    {"shellapi.h", "-lshell32"},
    {"commdlg.h", "-lcomdlg32"}
    // Add more headers and their corresponding flags as needed
};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Directory and batch file names
    std::string directory = ".project";
    std::string batchFileName = "compile.bat";

    // Get all .cpp files in the directory
    std::vector<std::string> cppFiles;
    GetCppFilesInDirectory(directory, cppFiles);

    // Extract includes from the .cpp files
    std::vector<std::string> includes;
    for (const auto& cppFile : cppFiles)
    {
        ExtractIncludesFromCppFile(cppFile, includes);
    }

    // Update the batch file with the .cpp files and the necessary flags
    UpdateBatchFile(batchFileName, cppFiles, headerFlags);

    MessageBox(NULL, "Batch file updated successfully!", "Info", MB_OK | MB_ICONINFORMATION);

    return 0;
}

void GetCppFilesInDirectory(const std::string& directory, std::vector<std::string>& cppFiles)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((directory + "\\*.cpp").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "Failed to read directory.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    do
    {
        std::string filePath = directory + "\\" + findFileData.cFileName;
        cppFiles.push_back(filePath);
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}

void ExtractIncludesFromCppFile(const std::string& cppFileName, std::vector<std::string>& includes)
{
    std::ifstream file(cppFileName);
    std::string line;

    if (!file.is_open())
    {
        MessageBox(NULL, "Failed to open C++ file.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    while (std::getline(file, line))
    {
        if (line.find("#include") != std::string::npos)
        {
            // Extract the include file name
            std::size_t start = line.find("\"") + 1;
            std::size_t end = line.find("\"", start);
            if (start != std::string::npos && end != std::string::npos)
            {
                includes.push_back(line.substr(start, end - start));
            }
        }
    }
    file.close();
}

#include <iostream> // For debugging purposes

void UpdateBatchFile(const std::string& batchFileName, const std::vector<std::string>& cppFiles, const std::map<std::string, std::string>& headerFlags)
{
    std::ofstream file(batchFileName, std::ios::trunc);

    if (!file.is_open())
    {
        MessageBox(NULL, "Failed to open batch file.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Start with the batch file command
    file << "@echo off\n";
    file << "cd .project\n";

    // Start with g++ compilation command
    file << "g++ -o build.exe ";

    // Add the main source file directly after the output file name
    file << "\"Files/main.cpp\" ";

    // Collect unique linker flags based on header includes
    std::set<std::string> flags;
    for (const auto& include : headerFlags)
    {
        bool found = false;
        for (const auto& cppFile : cppFiles)
        {
            std::ifstream file(cppFile);
            std::string line;
            while (std::getline(file, line))
            {
                // Debug output for each line in the file
                std::cout << "Checking line: " << line << std::endl;
                
                if (line.find("#include \"" + include.first + "\"") != std::string::npos ||
                    line.find("#include <" + include.first + ">") != std::string::npos)
                {
                    flags.insert(include.second);
                    found = true;
                    break; // Break out of the inner loop
                }
            }
            if (found) break; // Break out of the outer loop
        }
    }

    // Debug output to verify flags being added
    std::cout << "Flags to be added: ";
    for (const auto& flag : flags)
    {
        std::cout << flag << " ";
    }
    std::cout << std::endl;

    // Add linker flags to the batch file
    for (const auto& flag : flags)
    {
        file << flag << " ";
    }

    // Add all other .cpp files to the batch file
    for (const auto& cppFile : cppFiles)
    {
        std::string fileName = cppFile.substr(cppFile.find_last_of("\\/") + 1); // Extract file name from path
        if (fileName != "main.cpp")  // Exclude main.cpp since it's already included
        {
            file << "\"Files/" << fileName << "\" ";
        }
    }

    // End the batch file script
    file << "\npause\n";

    file.close();
}