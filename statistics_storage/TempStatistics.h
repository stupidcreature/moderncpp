//---------------------------------------------------------------------------

#ifndef TempStatisticsH
#define TempStatisticsH

//#include "definitions.h"
#include <ctime>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

const unsigned int   cMAX_GRAPHS = 16; // bei Aenderungen hier muss CTempStatistics angepasst werden
typedef time_t       TDateTime;
typedef std::wstring UnicodeString;
typedef unsigned int DWORD;

typedef struct {
    double    T[cMAX_GRAPHS];
    TDateTime Timestamp;
} TEMPREC;
typedef std::deque<TEMPREC> TEMPSTAT;


class CTempStatistics {
    typedef std::map<UnicodeString, unsigned int> SerialToPlotIdMap_t;


public:
    CTempStatistics(int classID = 0);

    virtual ~CTempStatistics();

    double GetMinTemp();

    double GetMaxTemp();

    double GetMaxTempSinceStart() { return m_MaxTempSoFar; }

    double GetMaxTempSinceStartRaw() { return m_MaxTempSoFarRaw; }

    TEMPSTAT* GetTempStat() { return &m_dTempStat; }

    TEMPSTAT* GetTempStatRaw() { return &m_dTempStatRaw; }

    UnicodeString GetLegendTextForGraphIndex(unsigned int idx);

    void SetStatSize(size_t statsize = 200);

    void SetStatWidth(unsigned int statwidth);

    unsigned int GetStatWidth() { return m_nStatWidth; }

    void AddValue(double val1, double val2, double val3, double val4, double val5 = 0, double val6 = 0, double val7 = 0,
                  double val8 = 0,
                  double val9 = 0, double val10 = 0, double val11 = 0, double val12 = 0, double val13 = 0,
                  double val14 = 0, double val15 = 0, double val16 = 0);

    void AddValueWithTimestamp(TDateTime datetime, double val1, double val2, double val3, double val4, double val5 = 0,
                               double val6 = 0, double val7 = 0, double val8 = 0,
                               double val9 = 0, double val10 = 0, double val11 = 0, double val12 = 0, double val13 = 0,
                               double val14 = 0, double val15 = 0, double val16 = 0);

    void AddValueAndKey(double val1, UnicodeString key1, UnicodeString legend1, double val2, UnicodeString key2,
                        UnicodeString legend2, double val3, UnicodeString key3, UnicodeString legend3, double val4,
                        UnicodeString key4, UnicodeString legend4,
                        double val5, UnicodeString key5, UnicodeString legend5, double val6, UnicodeString key6,
                        UnicodeString legend6, double val7, UnicodeString key7, UnicodeString legend7, double val8,
                        UnicodeString key8, UnicodeString legend8,
                        double val9 = 0, UnicodeString key9 = 0, UnicodeString legend9 = 0, double val10 = 0,
                        UnicodeString key10 = 0, UnicodeString legend10 = 0, double val11 = 0,
                        UnicodeString key11 = 0, UnicodeString legend11 = 0, double val12 = 0,
                        UnicodeString key12 = 0, UnicodeString legend12 = 0,
                        double val13 = 0, UnicodeString key13 = 0, UnicodeString legend13 = 0, double val14 = 0,
                        UnicodeString key14 = 0, UnicodeString legend14 = 0, double val15 = 0,
                        UnicodeString key15 = 0, UnicodeString legend15 = 0, double val16 = 0,
                        UnicodeString key16 = 0, UnicodeString legend16 = 0);

    void DuplicateLastEntries();

    size_t Length() { return m_dTempStat.size(); }

    size_t Width() { return m_nStatWidth; }

    void Clear()
    {
        m_dTempStat.clear();
        m_dTempStatRaw.clear();
    }

    void ResetMaxValues();

    bool IsEmpty() { return m_dTempStat.empty(); }

    size_t GetUniqueKeyCount();

    void SetAllowZeroValues(bool bAllowZeros) { m_bAllowZeroValues = bAllowZeros; }

    bool GetAllowZeroValues() { return m_bAllowZeroValues; }

    bool GetValuesByTimestamp(TDateTime TimeStamp, TEMPREC& ret);

    bool GetFilteredValuesByTimestamp(TDateTime TimeStamp, TEMPREC& ret);

    bool GetValuesByIndex(DWORD Index, TEMPREC& ret);

    bool GetFilteredValuesByIndex(DWORD Index, TEMPREC& ret);

    void SetMaxIndexForMaximumCalculation(DWORD Index);

    //Anzahl der bei der Filterung der Statistik beachteten Werte festlegen (0 deaktiviert die Filterung)
    void SetFilterLength(size_t filterlength) { m_nFilterLength = filterlength; }

    void SetActive(bool active)
    {
        m_bActive = active;
        if (active)
            m_dwActiveCounter = 0;
    }

    bool GetActive() { return m_bActive; }

    void SetSkipZeroValuesForFiltering(bool val = true) { m_bSkipZeroValuesForFiltering = val; }

    SerialToPlotIdMap_t m_mSerialToPlotId;

protected:
    void Maintenance();

    virtual void PerformFiltering();

    virtual void UpdateMaxValue();

    void FixMapping(const UnicodeString& key1, const UnicodeString& key2, const UnicodeString& key3,
                    const UnicodeString& key4,
                    const UnicodeString& key5, const UnicodeString& key6, const UnicodeString& key7,
                    const UnicodeString& key8,
                    const UnicodeString& key9, const UnicodeString& key10, const UnicodeString& key11,
                    const UnicodeString& key12,
                    const UnicodeString& key13, const UnicodeString& key14, const UnicodeString& key15,
                    const UnicodeString& key16);

    void RemoveFromStatistics(int indextoremove);

    int FindIndexForTimestamp(const TDateTime TimeStamp);

    UnicodeString m_LegendTexts[cMAX_GRAPHS];

    TEMPSTAT m_dTempStat;
    TEMPSTAT m_dTempStatRaw;
    size_t   m_dStatSize;
    size_t   m_nStatWidth;
    size_t   m_nFilterLength;
    bool     m_bAllowZeroValues;
    DWORD    m_dwMaxIndexForMaximumCalculation;
    bool     m_bActive;
    DWORD    m_dwActiveCounter;
    bool     m_bSkipZeroValuesForFiltering;
    int      m_iClassID;

    double m_MaxTempSoFar;
    double m_MaxTempSoFarRaw;
};

//---------------------------------------------------------------------------
#endif
