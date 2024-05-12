#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

std::string StringToUnicode(const std::string& str);

/// <summary>
/// Function which converts symbols into their respective hexadecimal values
/// </summary>
/// <param name="cSymb">symbol we want to change</param>
/// <returns>string which contains transformed value</returns>
std::string ToHex(char cSymb) {
    const char hexChars[] = "0123456789abcdef";
    return { hexChars[(cSymb >> 12) & 0xF], hexChars[(cSymb >> 8) & 0xF], hexChars[(cSymb >> 4) & 0xF], hexChars[cSymb & 0xF] };
}

/// <summary>
/// Function for reading JSON file.
/// </summary>
/// <param name="fJSON">Name of the file we want to read.</param>
/// <returns>String, which contains all the content of JSON file.</returns>
std::string JSON_Read(const std::string& fJSON) {
    std::ifstream ifFile(fJSON);
    return ifFile.is_open() ? std::string(std::istreambuf_iterator<char>(ifFile), std::istreambuf_iterator<char>()) : "";
}

/// <summary>
/// Function to collect all strings enclosed in quotation marks from a JSON string
/// </summary>
/// <param name="str">The JSON string from which to collect strings</param>
/// <returns>An unordered_map containing the collected strings as keys</returns>
std::unordered_map<std::string, std::string> CollectStrings(const std::string& str) {
    std::unordered_map<std::string, std::string> strings;
    std::string currentString;
    bool inQuotes = false;
    bool isKey = false;
    for (char c : str) {
        if (c == '"') {
            inQuotes = !inQuotes;
            if (!inQuotes) {
                if (isKey) {
                    strings.emplace(std::move(currentString), "");
                }
                else {
                    strings[currentString] = StringToUnicode(currentString);
                }
                currentString.clear();
                isKey = !isKey;
            }
        }
        else if (inQuotes) {
            currentString.push_back(c);
        }
    }
    return strings;
}

/// <summary>
/// Function to convert a string to its Unicode escape sequence
/// </summary>
/// <param name="str">The string to convert</param>
/// <returns>The Unicode escape sequence string</returns>
std::string StringToUnicode(const std::string& str) {
    std::string unicodeStr;
    for (char c : str) {
        unicodeStr += "\\u" + ToHex(c);
    }
    return unicodeStr;
}

/// <summary>
/// Function to write the replacement map to a file with modified Unicode escape sequence format
/// </summary>
/// <param name="replacementMap">The replacement map to write</param>
/// <param name="filename">The name of the file to write to</param>
void WriteReplacementMap(const std::unordered_map<std::string, std::string>& replacementMap, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& pair : replacementMap) {
            std::string modifiedValue = pair.second;
            size_t pos = 0;
            while ((pos = modifiedValue.find("\\u00", pos)) != std::string::npos) {
                modifiedValue.replace(pos, 6, "\\x" + modifiedValue.substr(pos + 4, 2));
                pos += 4;
            }
            file << pair.first << " -> " << modifiedValue << std::endl;
        }
    }
}

/// <summary>
/// Function to write the modified JSON with Unicode escape sequences back into the file
/// </summary>
/// <param name="filename">The name of the file to write to</param>
/// <param name="modifiedJson">The modified JSON string to write</param>
void WriteJSONToFile(const std::string& filename, const std::string& modifiedJson) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << modifiedJson;
        std::cout << "Modified JSON written back to file: " << filename << std::endl;
    }
    else {
        std::cerr << "Error: Unable to write modified JSON to file." << std::endl;
    }
}

int main() {
    std::string filename;
    std::cout << "Enter the name of the JSON file: ";
    std::cin >> filename;

    std::string originalJson = JSON_Read(filename);
    if (originalJson.empty()) {
        std::cerr << "Error: Unable to read JSON from file." << std::endl;
        return 1;
    }

    std::cout << "Read JSON from file:\n" << originalJson << std::endl;

    auto strings = CollectStrings(originalJson);
    std::unordered_map<std::string, std::string> replacementMap;
    for (const auto& str : strings) {
        replacementMap[str.first] = StringToUnicode(str.first);
    }

    std::string modifiedJson = originalJson;
    for (const auto& pair : replacementMap) {
        size_t pos = modifiedJson.find(pair.first);
        while (pos != std::string::npos) {
            modifiedJson.replace(pos, pair.first.length(), pair.second);
            pos = modifiedJson.find(pair.first, pos + pair.second.length());
            if (pos == std::string::npos) {
                break;
            }
            if (pair.first == pair.second) {
                break;
            }
        }
    }

    // Write the replacement map to a file
    std::string mapFilename = "replacement_map.txt";
    WriteReplacementMap(replacementMap, mapFilename);
    std::cout << "Replacement map written to " << mapFilename << std::endl;

    std::cout << "Unicode representation:\n" << modifiedJson << std::endl;

    WriteJSONToFile(filename, modifiedJson);

    return 0;
}