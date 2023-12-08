#pragma once

#include <map>
#include <string>
#include <vector>

using namespace std;
namespace PanoSlicer
{
    class CConfigParser
    {
    public:
        CConfigParser();
        ~CConfigParser();

    public:
        bool Parser(const string & cFilePath);
        bool HasSection(const string & cSection);
        int GetSections(vector<string> & vecSections);
        int GetKeys(const string & strSection, vector<string> & vec);
        const map<string, string> * GetSectionConfig(const string & cSection);
        string GetConfig(const string & cSection, const string & cKey);
        string GetDefConfig(const string & cSection, const string & cKey, const string & cDef);

    private:
        map<string, map<string, string> *> m_mpConfigData;
    };

}
