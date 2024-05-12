#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

std::string ToHex(char cSymb);
std::string JSON_Read(const std::string& fJSON);
std::unordered_map<std::string, std::string> CollectStrings(const std::string& sContent);
std::string StringToUnicode(const std::string& str);
void WriteReplacementMap(const std::unordered_map<std::string, std::string>& u_mReplacementMap, const std::string& sFilename);
void WriteJSONToFile(const std::string& sFilename, const std::string& sModJSONName);

/// <summary>
/// Function which converts symbols into their respective hexadecimal values
/// </summary>
/// <param name="cSymb">symbol we want to change</param>
/// <returns>string which contains transformed value</returns>
std::string ToHex(char cSymb) {
    const char cHexArr[] = "0123456789abcdef";
    return { cHexArr[(cSymb >> 12) & 0xF], cHexArr[(cSymb >> 8) & 0xF], cHexArr[(cSymb >> 4) & 0xF], cHexArr[cSymb & 0xF] };
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
/// <param name="sContent">The JSON content string from which to collect strings</param>
/// <returns>An unordered_map containing the collected strings as keys</returns>
std::unordered_map<std::string, std::string> CollectStrings(const std::string& sContent) {
    std::unordered_map<std::string, std::string> umStrings;
    std::string sCurrent;
    bool bInQuotes = false;
    bool bIsKey = false;
    for (char cTemp : sContent) {
        if (cTemp == '"') {
            bInQuotes = !bInQuotes;
            if (!bInQuotes) {
                if (bIsKey) {
                    umStrings.emplace(std::move(sCurrent), "");
                }
                else {
                    umStrings[sCurrent] = StringToUnicode(sCurrent);
                }
                sCurrent.clear();
                bIsKey = !bIsKey;
            }
        }
        else if (bInQuotes) {
            sCurrent.push_back(cTemp);
        }
    }
    return umStrings;
}

/// <summary>
/// Function to convert a string to its Unicode escape sequence
/// </summary>
/// <param name="sToConvert">The string to convert into Unicode</param>
/// <returns>The Unicode escape sequence string</returns>
std::string StringToUnicode(const std::string& sToConvert) {
    std::string sConverted;
    for (char cTemp : sToConvert) {
        sConverted += "\\u" + ToHex(cTemp);
    }
    return sConverted;
}

/// <summary>
/// Function to write the replacement map to a file with modified Unicode escape sequence format
/// </summary>
/// <param name="u_mReplacementMap">The replacement map to write</param>
/// <param name="sFilename">The name of the file to write to</param>
void WriteReplacementMap(const std::unordered_map<std::string, std::string>& u_mReplacementMap, const std::string& sFilename) {
    std::ofstream ofstrFile(sFilename);
    if (ofstrFile.is_open()) {
        for (const auto& sPair : u_mReplacementMap) {
            std::string sModified = sPair.second;
            size_t s_tPos = 0;
            while ((s_tPos = sModified.find("\\u00", s_tPos)) != std::string::npos) {
                sModified.replace(s_tPos, 6, "\\x" + sModified.substr(s_tPos + 4, 2));
                s_tPos += 4;
            }
            ofstrFile << sPair.first << " -> " << sModified << std::endl;
        }
    }
}

/// <summary>
/// Function to write the modified JSON with Unicode escape sequences back into the file
/// </summary>
/// <param name="sFilename">The name of the file to write to</param>
/// <param name="sModJSONName">The modified JSON string to write</param>
void WriteJSONToFile(const std::string& sFilename, const std::string& sModJSONName) {
    std::ofstream ofstrFile(sFilename);
    if (ofstrFile.is_open()) {
        ofstrFile << sModJSONName;
        std::cout << "Modified JSON written back to file: " << sFilename << std::endl;
    }
    else {
        std::cerr << "Error: Unable to write modified JSON to file." << std::endl;
    }
}

int main() {
    std::string sFilename;
    std::cout << "Enter the name of the JSON file: ";
    std::cin >> sFilename;

    std::string sJSONName = JSON_Read(sFilename);
    if (sJSONName.empty()) {
        std::cerr << "Error: Unable to read JSON from file." << std::endl;
        return 1;
    }

    std::cout << "Read JSON from file:\n" << sJSONName << std::endl;

    auto u_mStrings = CollectStrings(sJSONName);
    std::unordered_map<std::string, std::string> u_mReplacement;
    for (const auto& sTemp : u_mStrings) {
        u_mReplacement[sTemp.first] = StringToUnicode(sTemp.first);
    }

    std::string sModJSONName = sJSONName;
    /*for (const auto& pair : replacementMap) {
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
    }*/
    for (const auto& sPair : u_mReplacement) {
        std::string::size_type s_zPos = sModJSONName.find(sPair.first);
        while (s_zPos != std::string::npos) {
            if (s_zPos == 0 || sModJSONName[s_zPos - 1] == '"') {
                std::string::size_type endPos = s_zPos + sPair.first.length();
                if (endPos == sModJSONName.length() || sModJSONName[endPos] == '"') {
                    sModJSONName.replace(s_zPos, sPair.first.length(), sPair.second);
                }
            }
            s_zPos = sModJSONName.find(sPair.first, s_zPos + sPair.second.length());
            if (s_zPos == std::string::npos) {
                break;
            }
            if (sPair.first == sPair.second) {
                break;
            }
        }
    }
  

    std::string sMapFilename = "replacement_map.txt";
    WriteReplacementMap(u_mReplacement, sMapFilename);
    std::cout << "Replacement map written to " << sMapFilename << std::endl;

    std::cout << "Unicode representation:\n" << sModJSONName << std::endl;

    WriteJSONToFile(sFilename, sModJSONName);

    return 0;
}