#include <iostream>
#include "TempStatistics.h"
#include "StatisticsStorage.h"


const unsigned int cMAX_DISKS = 32;

typedef struct {
    double Temperature;
//    char ModelKey[48];
//    char ModelStr[48];
    UnicodeString ModelKey;
    UnicodeString ModelStr;
} HDDSMARTTEMP_t;

typedef struct {
    DWORD HDD_Mask;
    DWORD HDD_TempMask;
    DWORD HDD_TempMask_WithoutUSB;
    DWORD HDD_SMARTSupportedMask;
    DWORD HDD_AvailableAndDisabledMask;
    DWORD HDD_SleepMask;
    HDDSMARTTEMP_t HDD_Temps[cMAX_DISKS];            // die Statistik unterstützt im Moment nur cMAX_GRAPHS Temperaturen
} AM_MD_DISKDATA;

typedef struct {
    AM_MD_DISKDATA DiskData;

} AM_MEASURED_DATA;
AM_MEASURED_DATA m_MD;


int main() {

    // prepare mock data
    for (size_t i = 0; i < cMAX_DISKS; ++i) {
        m_MD.DiskData.HDD_Temps[i].ModelKey = L"KEY_";
        m_MD.DiskData.HDD_Temps[i].ModelKey.append(std::to_wstring(i));
        m_MD.DiskData.HDD_Temps[i].ModelStr = L"MODELSTRING";
        m_MD.DiskData.HDD_Temps[i].Temperature = 20+i*2;
    }


    CStatisticsStorage new_stat(5, cMAX_DISKS, 5, 3);
    std::vector<double> values = {1,2,3,5,4};

    std::vector<std::wstring> keys;
    keys.push_back(L"Model 1");
    keys.push_back(L"Model 2");
    keys.push_back(L"Model 3");
    keys.push_back(L"Model 4");
    keys.push_back(L"Model 5");

    std::vector<std::wstring> legend;
    legend.push_back(L"Legend 1");
    legend.push_back(L"Legend 2");
    legend.push_back(L"Legend 3");
    legend.push_back(L"Legend 4");
    legend.push_back(L"Legend 5");


    new_stat.AddValuesWithKeys(values, keys, legend);
    new_stat.AddValuesWithKeys(values, keys, legend);

//    new_stat.AddValues(values);
//    new_stat.AddValues(values);
//    new_stat.AddValues(values);
//    new_stat.AddValues(values);
//    new_stat.AddValues(values);
//    new_stat.AddValues(values);
//    new_stat.AddValues(values);
//    new_stat.AddValues(values);
//    new_stat.AddValues(values);


    CTempStatistics old_stat;
    old_stat.SetStatSize(200);
    old_stat.SetFilterLength(5);



    for (int i = 0; i < 10000; ++i) {

        old_stat.AddValueAndKey(m_MD.DiskData.HDD_Temps[0].Temperature, m_MD.DiskData.HDD_Temps[0].ModelKey,
                               m_MD.DiskData.HDD_Temps[0].ModelStr,
                               m_MD.DiskData.HDD_Temps[1].Temperature, m_MD.DiskData.HDD_Temps[1].ModelKey,
                               m_MD.DiskData.HDD_Temps[1].ModelStr,
                               m_MD.DiskData.HDD_Temps[2].Temperature, m_MD.DiskData.HDD_Temps[2].ModelKey,
                               m_MD.DiskData.HDD_Temps[2].ModelStr,
                               m_MD.DiskData.HDD_Temps[3].Temperature, m_MD.DiskData.HDD_Temps[3].ModelKey,
                               m_MD.DiskData.HDD_Temps[3].ModelStr,
                               m_MD.DiskData.HDD_Temps[4].Temperature, m_MD.DiskData.HDD_Temps[4].ModelKey,
                               m_MD.DiskData.HDD_Temps[4].ModelStr,
                               m_MD.DiskData.HDD_Temps[5].Temperature, m_MD.DiskData.HDD_Temps[5].ModelKey,
                               m_MD.DiskData.HDD_Temps[5].ModelStr,
                               m_MD.DiskData.HDD_Temps[6].Temperature, m_MD.DiskData.HDD_Temps[6].ModelKey,
                               m_MD.DiskData.HDD_Temps[6].ModelStr,
                               m_MD.DiskData.HDD_Temps[7].Temperature, m_MD.DiskData.HDD_Temps[7].ModelKey,
                               m_MD.DiskData.HDD_Temps[7].ModelStr,
                               m_MD.DiskData.HDD_Temps[8].Temperature, m_MD.DiskData.HDD_Temps[8].ModelKey,
                               m_MD.DiskData.HDD_Temps[8].ModelStr,
                               m_MD.DiskData.HDD_Temps[9].Temperature, m_MD.DiskData.HDD_Temps[9].ModelKey,
                               m_MD.DiskData.HDD_Temps[9].ModelStr,
                               m_MD.DiskData.HDD_Temps[10].Temperature, m_MD.DiskData.HDD_Temps[10].ModelKey,
                               m_MD.DiskData.HDD_Temps[10].ModelStr,
                               m_MD.DiskData.HDD_Temps[11].Temperature, m_MD.DiskData.HDD_Temps[11].ModelKey,
                               m_MD.DiskData.HDD_Temps[11].ModelStr,
                               m_MD.DiskData.HDD_Temps[12].Temperature, m_MD.DiskData.HDD_Temps[12].ModelKey,
                               m_MD.DiskData.HDD_Temps[12].ModelStr,
                               m_MD.DiskData.HDD_Temps[13].Temperature, m_MD.DiskData.HDD_Temps[13].ModelKey,
                               m_MD.DiskData.HDD_Temps[13].ModelStr,
                               m_MD.DiskData.HDD_Temps[14].Temperature, m_MD.DiskData.HDD_Temps[14].ModelKey,
                               m_MD.DiskData.HDD_Temps[14].ModelStr,
                               m_MD.DiskData.HDD_Temps[15].Temperature, m_MD.DiskData.HDD_Temps[15].ModelKey,
                               m_MD.DiskData.HDD_Temps[15].ModelStr
        );
    }

    old_stat.DuplicateLastEntries();


    return 0;
}