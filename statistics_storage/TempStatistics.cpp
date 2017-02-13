//---------------------------------------------------------------------------


#pragma hdrstop

#include "TempStatistics.h"
//#include "util_func.h"
#include <math.h>


#pragma package(smart_init)


/* memset example */
#include <stdio.h>
#include <string.h>
void ZeroMemory(void* addr, size_t size) { memset(addr, 0, size); }


bool Min(const int a, const int b) { return a < b ? a : b; }

CTempStatistics::CTempStatistics(int classID /*= 0*/)
{
    m_dTempStat.clear();
    m_dTempStatRaw.clear();
    m_dStatSize  = 0;
    m_nStatWidth = 0;
    m_mSerialToPlotId.clear();
    m_nFilterLength                   = 0;
    m_MaxTempSoFar                    = 0;
    m_MaxTempSoFarRaw                 = 0;
    m_dwMaxIndexForMaximumCalculation = cMAX_GRAPHS - 1;
    m_bAllowZeroValues                = false;
    m_bActive                         = true;
    m_dwActiveCounter                 = 0;
    m_bSkipZeroValuesForFiltering     = false;
    m_iClassID                        = classID;

    //default Statistikgroesse setzen
    SetStatSize();
}
//---------------------------------------------------------------------------

CTempStatistics::~CTempStatistics()
{
    m_dTempStat.clear();
    m_dTempStatRaw.clear();
}
//---------------------------------------------------------------------------

double CTempStatistics::GetMinTemp()
{
    TEMPSTAT::iterator it  = m_dTempStat.begin();
    double             ret = 99999999.9;

    if (!m_bAllowZeroValues) {
        while (it != m_dTempStat.end()) {
            for (unsigned int i = 0; i < cMAX_GRAPHS; i++) {
                if (((*it).T[i] < ret) && ((*it).T[i] > 0))
                    ret = (*it).T[i];
            }
            it++;
        }
    } else {
        while (it != m_dTempStat.end()) {
            for (unsigned int i = 0; i < cMAX_GRAPHS; i++) {
                if ((*it).T[i] < ret)
                    ret = (*it).T[i];
            }
            it++;
        }
    }

    return ret;
}
//---------------------------------------------------------------------------

double CTempStatistics::GetMaxTemp()
{
    TEMPSTAT::iterator it  = m_dTempStat.begin();
    double             ret = 0;

    while (it != m_dTempStat.end()) {
        for (unsigned int i = 0; i < cMAX_GRAPHS; i++) {
            if ((*it).T[i] > ret)
                ret = (*it).T[i];
        }
        it++;
    }
    return ret;
}
//---------------------------------------------------------------------------


void CTempStatistics::SetStatSize(size_t newsize)
{
    m_dStatSize = newsize;
    Maintenance();
}
//---------------------------------------------------------------------------


void CTempStatistics::UpdateMaxValue()
{
    size_t statsize = m_dTempStat.size();
    if (statsize < 1)
        return;

    TEMPREC tr = m_dTempStat[statsize - 1];
    for (int i = 0; i < Min((int)m_dwMaxIndexForMaximumCalculation + 1, cMAX_GRAPHS); i++)
        if (m_MaxTempSoFar < tr.T[i])
            m_MaxTempSoFar = tr.T[i];


    statsize = m_dTempStatRaw.size();
    //die statsize sollte gleich gross sein
    if (statsize < 1)
        return;

    tr = m_dTempStatRaw[statsize - 1];
    for (int i = 0; i < Min((int)m_dwMaxIndexForMaximumCalculation + 1, cMAX_GRAPHS); i++)
        if (m_MaxTempSoFarRaw < tr.T[i])
            m_MaxTempSoFarRaw = tr.T[i];
}
//---------------------------------------------------------------------------


void CTempStatistics::AddValueWithTimestamp(TDateTime datetime, double val1, double val2, double val3, double val4,
                                            double val5, double val6, double val7, double val8,
                                            double val9, double val10, double val11, double val12, double val13,
                                            double val14, double val15, double val16)
{
    TEMPREC tr;

    tr.T[0]  = val1;
    tr.T[1]  = val2;
    tr.T[2]  = val3;
    tr.T[3]  = val4;
    tr.T[4]  = val5;
    tr.T[5]  = val6;
    tr.T[6]  = val7;
    tr.T[7]  = val8;
    tr.T[8]  = val9;
    tr.T[9]  = val10;
    tr.T[10] = val11;
    tr.T[11] = val12;
    tr.T[12] = val13;
    tr.T[13] = val14;
    tr.T[14] = val15;
    tr.T[15] = val16;

    tr.Timestamp = datetime;

    m_dTempStatRaw.push_back(tr);
    PerformFiltering();
    Maintenance();

    //maximale Temperatur updaten (nach evtl. vorangegangener Filterung)
    UpdateMaxValue();
}


void CTempStatistics::AddValue(double val1, double val2, double val3, double val4, double val5, double val6, double val7,
                               double val8,
                               double val9, double val10, double val11, double val12, double val13, double val14,
                               double val15, double val16)
{
    TEMPREC tr;

    tr.T[0]  = val1;
    tr.T[1]  = val2;
    tr.T[2]  = val3;
    tr.T[3]  = val4;
    tr.T[4]  = val5;
    tr.T[5]  = val6;
    tr.T[6]  = val7;
    tr.T[7]  = val8;
    tr.T[8]  = val9;
    tr.T[9]  = val10;
    tr.T[10] = val11;
    tr.T[11] = val12;
    tr.T[12] = val13;
    tr.T[13] = val14;
    tr.T[14] = val15;
    tr.T[15] = val16;

    time(&tr.Timestamp);

    m_dTempStatRaw.push_back(tr);
    PerformFiltering();
    Maintenance();

    //maximale Temperatur updaten (nach evtl. vorangegangener Filterung)
    UpdateMaxValue();
}
//---------------------------------------------------------------------------


//Lieft den Key, aber ohne den Zusatz '#(DriveX)'
UnicodeString CTempStatistics::GetLegendTextForGraphIndex(unsigned int idx)
{
    if (idx >= cMAX_GRAPHS)
        return UnicodeString(L"-");
    else
        return m_LegendTexts[idx];

    //  SerialToPlotIdMap_t::iterator it = m_mSerialToPlotId.begin();
    //  while( it != m_mSerialToPlotId.end() )
    //  {
    //    if( (*it).second == idx ) {
    //      int idx = (*it).first.LastDelimiter("#");
    //      if(!idx)
    //	return (*it).first;
    //      else
    //	return (*it).first.SubString(1, idx-1);
    //    }
    //    it++;
    //  }
    //
    //  return "-";
}
//---------------------------------------------------------------------------


void CTempStatistics::RemoveFromStatistics(int indextoremove)
{
    TEMPSTAT::iterator it = m_dTempStat.begin();
    while (it != m_dTempStat.end()) {
        for (int i = indextoremove; i < cMAX_GRAPHS - 2; i++) {
            if (i > 0)
                (*it).T[i] = (*it).T[i + 1];
        }
        (*it).T[cMAX_GRAPHS - 1] = 0;
        it++;
    }
    //m_nStatWidth--; // m�sste eigentlich rein. Wie wird die Breite angepasset, wenn Werte verschwinden?
}
//---------------------------------------------------------------------------


void CTempStatistics::FixMapping(const UnicodeString& key1, const UnicodeString& key2, const UnicodeString& key3,
                                 const UnicodeString& key4,
                                 const UnicodeString& key5, const UnicodeString& key6, const UnicodeString& key7,
                                 const UnicodeString& key8,
                                 const UnicodeString& key9, const UnicodeString& key10, const UnicodeString& key11,
                                 const UnicodeString& key12,
                                 const UnicodeString& key13, const UnicodeString& key14, const UnicodeString& key15,
                                 const UnicodeString& key16)
{
    //herausfinden, welche keys neu (N) sind und welche nicht mehr dabei sind (R)
    //fuer alle R -> n�chste Statistikspalte in alte Rte Spalte verschieben
    //
    //fuer alle N->
    //erste freie Spalte finden und als Index fuer diesen Key einfuegen

    //auf neue Keys pruefen
    std::vector<UnicodeString> newkeys;
    newkeys.clear();
    if (!key1.empty() && (m_mSerialToPlotId.find(key1) == m_mSerialToPlotId.end()))
        newkeys.push_back(key1);
    if (!key2.empty() && (m_mSerialToPlotId.find(key2) == m_mSerialToPlotId.end()))
        newkeys.push_back(key2);
    if (!key3.empty() && (m_mSerialToPlotId.find(key3) == m_mSerialToPlotId.end()))
        newkeys.push_back(key3);
    if (!key4.empty() && (m_mSerialToPlotId.find(key4) == m_mSerialToPlotId.end()))
        newkeys.push_back(key4);
    if (!key5.empty() && (m_mSerialToPlotId.find(key5) == m_mSerialToPlotId.end()))
        newkeys.push_back(key5);
    if (!key6.empty() && (m_mSerialToPlotId.find(key6) == m_mSerialToPlotId.end()))
        newkeys.push_back(key6);
    if (!key7.empty() && (m_mSerialToPlotId.find(key7) == m_mSerialToPlotId.end()))
        newkeys.push_back(key7);
    if (!key8.empty() && (m_mSerialToPlotId.find(key8) == m_mSerialToPlotId.end()))
        newkeys.push_back(key8);
    if (!key9.empty() && (m_mSerialToPlotId.find(key9) == m_mSerialToPlotId.end()))
        newkeys.push_back(key9);
    if (!key10.empty() && (m_mSerialToPlotId.find(key10) == m_mSerialToPlotId.end()))
        newkeys.push_back(key10);
    if (!key11.empty() && (m_mSerialToPlotId.find(key11) == m_mSerialToPlotId.end()))
        newkeys.push_back(key11);
    if (!key12.empty() && (m_mSerialToPlotId.find(key12) == m_mSerialToPlotId.end()))
        newkeys.push_back(key12);
    if (!key13.empty() && (m_mSerialToPlotId.find(key13) == m_mSerialToPlotId.end()))
        newkeys.push_back(key13);
    if (!key14.empty() && (m_mSerialToPlotId.find(key14) == m_mSerialToPlotId.end()))
        newkeys.push_back(key14);
    if (!key15.empty() && (m_mSerialToPlotId.find(key15) == m_mSerialToPlotId.end()))
        newkeys.push_back(key15);
    if (!key16.empty() && (m_mSerialToPlotId.find(key16) == m_mSerialToPlotId.end()))
        newkeys.push_back(key16);

    //Liste um fehlende Keys zu finden
    std::set<UnicodeString> missingkeys;
    missingkeys.clear();

    //kleinsten (nicht belegten) Index bestimmen und alle vorhandenen Keys ind missingkeys-Liste aufnehmen
    unsigned int                  newindex = 0;
    SerialToPlotIdMap_t::iterator it       = m_mSerialToPlotId.begin();
    if (it == m_mSerialToPlotId.end()) {
        newindex = 0;
    } else {
        //es sind schon Keys vorhanden -- diese nebenbei in Liste missingkeys aufnehmen
        while (it != m_mSerialToPlotId.end()) {
            if ((*it).second >= newindex)
                newindex = (*it).second + 1;
            missingkeys.insert((*it).first);
            it++;
        }
    }

    //und nun alle �bergebenen aus missingkeys loeschen
    missingkeys.erase(key1);
    missingkeys.erase(key2);
    missingkeys.erase(key3);
    missingkeys.erase(key4);
    missingkeys.erase(key5);
    missingkeys.erase(key6);
    missingkeys.erase(key7);
    missingkeys.erase(key8);
    missingkeys.erase(key9);
    missingkeys.erase(key10);
    missingkeys.erase(key11);
    missingkeys.erase(key12);
    missingkeys.erase(key13);
    missingkeys.erase(key14);
    missingkeys.erase(key15);
    missingkeys.erase(key16);

    //in missingkeys verbleibende Keys existieren nicht mehr und muessen aus der Statistik entfernt werden.
    std::set<UnicodeString>::iterator lit = missingkeys.begin();
    while (lit != missingkeys.end()) {
        it = m_mSerialToPlotId.find(*lit);
        if (it != m_mSerialToPlotId.end()) {
            RemoveFromStatistics((*it).second);
            if (newindex)
                newindex--; //da nun ein Eintrag weniger in der Statistik ist
            int oldindex = it->second;
            m_mSerialToPlotId.erase(it);

            //der Key ist geloescht, aber nun sind alle IDs die vorher groesser waren als der geloeschte falsch
            //und muessen um eins dekrementiert werden
            SerialToPlotIdMap_t::iterator it2 = m_mSerialToPlotId.begin();
            while (it2 != m_mSerialToPlotId.end()) {
                if (it2->second > oldindex)
                    it2->second--;
                it2++;
            }
        }
        lit++;
    }

    //solange Platz im Array ist -> neue Werte dem Array hinzufuegen
    std::vector<UnicodeString>::iterator newkeyit = newkeys.begin();
    while (newkeyit != newkeys.end()) {
        if (newindex <= cMAX_GRAPHS - 1)
            m_mSerialToPlotId[(*newkeyit)] = newindex++;
        newkeyit++;
    }
}
//---------------------------------------------------------------------------


void CTempStatistics::AddValueAndKey(double val1, UnicodeString key1, UnicodeString legend1,
                                     double val2, UnicodeString key2, UnicodeString legend2,
                                     double val3, UnicodeString key3, UnicodeString legend3,
                                     double val4, UnicodeString key4, UnicodeString legend4,
                                     double val5, UnicodeString key5, UnicodeString legend5,
                                     double val6, UnicodeString key6, UnicodeString legend6,
                                     double val7, UnicodeString key7, UnicodeString legend7,
                                     double val8, UnicodeString key8, UnicodeString legend8,
                                     double val9, UnicodeString key9, UnicodeString legend9,
                                     double val10, UnicodeString key10, UnicodeString legend10,
                                     double val11, UnicodeString key11, UnicodeString legend11,
                                     double val12, UnicodeString key12, UnicodeString legend12,
                                     double val13, UnicodeString key13, UnicodeString legend13,
                                     double val14, UnicodeString key14, UnicodeString legend14,
                                     double val15, UnicodeString key15, UnicodeString legend15,
                                     double val16, UnicodeString key16, UnicodeString legend16)
{
    TEMPREC tr;
    ZeroMemory(&tr, sizeof(TEMPREC));

    FixMapping(key1, key2, key3, key4, key5, key6, key7, key8, key9, key10, key11, key12, key13, key14, key15, key16);

    SerialToPlotIdMap_t::iterator it;

    if (!key1.empty()) {
        it = m_mSerialToPlotId.find(key1);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val1;
            m_LegendTexts[(*it).second] = legend1;
        }
    }

    if (!key2.empty()) {
        it = m_mSerialToPlotId.find(key2);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val2;
            m_LegendTexts[(*it).second] = legend2;
        }
    }

    if (!key3.empty()) {
        it = m_mSerialToPlotId.find(key3);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val3;
            m_LegendTexts[(*it).second] = legend3;
        }
    }

    if (!key4.empty()) {
        it = m_mSerialToPlotId.find(key4);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val4;
            m_LegendTexts[(*it).second] = legend4;
        }
    }

    if (!key5.empty()) {
        it = m_mSerialToPlotId.find(key5);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val5;
            m_LegendTexts[(*it).second] = legend5;
        }
    }

    if (!key6.empty()) {
        it = m_mSerialToPlotId.find(key6);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val6;
            m_LegendTexts[(*it).second] = legend6;
        }
    }

    if (!key7.empty()) {
        it = m_mSerialToPlotId.find(key7);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val7;
            m_LegendTexts[(*it).second] = legend7;
        }
    }

    if (!key8.empty()) {
        it = m_mSerialToPlotId.find(key8);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val8;
            m_LegendTexts[(*it).second] = legend8;
        }
    }

    if (!key9.empty()) {
        it = m_mSerialToPlotId.find(key9);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val9;
            m_LegendTexts[(*it).second] = legend9;
        }
    }

    if (!key10.empty()) {
        it = m_mSerialToPlotId.find(key10);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val10;
            m_LegendTexts[(*it).second] = legend10;
        }
    }

    if (!key11.empty()) {
        it = m_mSerialToPlotId.find(key11);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val11;
            m_LegendTexts[(*it).second] = legend11;
        }
    }

    if (!key12.empty()) {
        it = m_mSerialToPlotId.find(key12);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val12;
            m_LegendTexts[(*it).second] = legend12;
        }
    }

    if (!key13.empty()) {
        it = m_mSerialToPlotId.find(key13);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val13;
            m_LegendTexts[(*it).second] = legend13;
        }
    }

    if (!key14.empty()) {
        it = m_mSerialToPlotId.find(key14);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val14;
            m_LegendTexts[(*it).second] = legend14;
        }
    }

    if (!key15.empty()) {
        it = m_mSerialToPlotId.find(key15);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val15;
            m_LegendTexts[(*it).second] = legend15;
        }
    }

    if (!key16.empty()) {
        it = m_mSerialToPlotId.find(key16);
        if (it != m_mSerialToPlotId.end()) {
            tr.T[(*it).second]          = val16;
            m_LegendTexts[(*it).second] = legend16;
        }
    }

    time(&tr.Timestamp);

    m_dTempStatRaw.push_back(tr);
    PerformFiltering();
    Maintenance();

    //maximale Temperatur updaten (nach evtl. vorangegangener Filterung)
    UpdateMaxValue();
}
//---------------------------------------------------------------------------


void CTempStatistics::DuplicateLastEntries()
{
    if (m_dTempStatRaw.size() == 0)
        return;

    TEMPREC tr = m_dTempStatRaw[m_dTempStatRaw.size() - 1];

    m_dTempStatRaw.push_back(tr);
    PerformFiltering();
    Maintenance();
}
//---------------------------------------------------------------------------


void CTempStatistics::Maintenance()
{
    while (m_dTempStat.size() > m_dStatSize) {
        m_dTempStat.pop_front();
        m_dTempStatRaw.pop_front();
    }

    //!!!Nullwerte vom Ende der Statistik entfernen (sollten eigentlich nie eingefuegt werden)
    size_t i = m_dTempStat.size();
    if (!m_bAllowZeroValues) {
        while (i) {
            if (!(m_dTempStat[i - 1].T[0] || m_dTempStat[i - 1].T[1] || m_dTempStat[i - 1].T[2] || m_dTempStat[i - 1].T[3]
                  || m_dTempStat[i - 1].T[4] || m_dTempStat[i - 1].T[5] || m_dTempStat[i - 1].T[6] || m_dTempStat[i - 1].T[7]
                  || m_dTempStat[i - 1].T[8] || m_dTempStat[i - 1].T[9] || m_dTempStat[i - 1].T[10] || m_dTempStat[i - 1].T[11]
                  || m_dTempStat[i - 1].T[12] || m_dTempStat[i - 1].T[13] || m_dTempStat[i - 1].T[14] || m_dTempStat[i - 1].T[15])) {
                m_dTempStat.pop_back();
                m_dTempStatRaw.pop_back();
                i--;
            } else
                break;
        }
    }
}
//---------------------------------------------------------------------------


void CTempStatistics::SetStatWidth(unsigned int statwidth)
{
    if (statwidth > cMAX_GRAPHS)
        m_nStatWidth = cMAX_GRAPHS;
    else
        m_nStatWidth = statwidth;
}
//---------------------------------------------------------------------------


void CTempStatistics::PerformFiltering()
{

    size_t valuecount = m_dTempStatRaw.size();

    //dies duerfte eigentlich nie passieren, da vor dem Aufruf dieser
    //Funktion immer ein Wert an m_dTempStatRaw angehaengt wird
    if (valuecount < 1)
        return;

    //neuen Wert an die Statistik anhaengen
    //(dieser wird ueberschrieben, falls noch gefiltert wird (bei m_nFilterLength>0))
    m_dTempStat.push_back(m_dTempStatRaw[valuecount - 1]);

    if (!m_nFilterLength) {
        return;
    }

    // bei deaktivierter GPU moeglichst steile 'Aus-Flanke'
    // -> keine Filterung
    if (!m_bActive) {
        return;
    } else {
        m_dwActiveCounter++;
    }

    size_t fcount = (valuecount < m_nFilterLength) ? (valuecount) : (m_nFilterLength);

    // bei gerade aktivierter GPU moeglichst steile 'An-Flanke'
    // -> Filterung nicht fuer m_nFilterLength sondern nur fuer die Anzahl der Werte nach der Aktivierung durchfuehren
    // m_bSkipZeroValuesForFiltering muss dafuer true sein
    if (m_dwActiveCounter < fcount)
        fcount = m_dwActiveCounter;
    if (fcount <= 0) {
        return;
    }

    //#ifdef _DEBUG
    //  if (m_iClassID == 100) {                                      // GPU0
    //    UnicodeString u;
    //    u.sprintf(L"Filter: %d", fcount);
    //    OutputDebugString(u.w_str());
    //  }
    //#endif

    for (size_t i = 0; i < m_nStatWidth; i++) {
        double fval     = 0;
        size_t sumCount = 0;
        for (size_t j = 0; j < fcount; j++) {
            //#ifdef _DEBUG
            //if( (m_iClassID == 100) && (m_dTempStatRaw[valuecount-j-1].T[i] == 0) ) DebugBreak();
            //#endif
            if (!m_bSkipZeroValuesForFiltering) {
                fval += m_dTempStatRaw[valuecount - j - 1].T[i];
                sumCount++;
            } else {
                if (m_dTempStatRaw[valuecount - j - 1].T[i] > 0) {
                    fval += m_dTempStatRaw[valuecount - j - 1].T[i];
                    sumCount++;
                }
            }
        }
        if (sumCount > 0)
            fval /= sumCount;
        m_dTempStat[valuecount - 1].T[i] = fval;
    }
}
//---------------------------------------------------------------------------

size_t CTempStatistics::GetUniqueKeyCount()
{
    return m_mSerialToPlotId.size();
}


//---------------------------------------------------------------------------
//eine binaere Suche nach dem Eintrag mit dem passenden TimeStamp
//Im Fehlerfall liefert die Funktion -1
int CTempStatistics::FindIndexForTimestamp(const TDateTime TimeStamp)
{
    DWORD counter = 0;
    int   left    = 0;
    int   right   = m_dTempStatRaw.size() - 1;
    if (right < 0)
        return -1; //bei leerer Statistik Fehler zurueckliefern

    while (left <= right) {
        //Abbrechen, wenn er hier haengen bleibt (evtl. falls die Uhrzeit verstellt wurde)
        if (++counter > 30)
            return -1;

        int mid = (left + right) / 2;
        if (m_dTempStatRaw[mid].Timestamp > TimeStamp)
            right = mid - 1;
        else
            left = mid + 1;
    }

    return right;
}

//---------------------------------------------------------------------------
bool CTempStatistics::GetValuesByTimestamp(TDateTime TimeStamp, TEMPREC& ret)
{
    int index = FindIndexForTimestamp(TimeStamp);
    if (index < 0)
        return false;

    ret = m_dTempStatRaw[index];

    return true;
}


//---------------------------------------------------------------------------
bool CTempStatistics::GetFilteredValuesByTimestamp(TDateTime TimeStamp, TEMPREC& ret)
{
    int index = FindIndexForTimestamp(TimeStamp);
    if (index < 0)
        return false;

    ret = m_dTempStat[index];

    return true;
}


//---------------------------------------------------------------------------
bool CTempStatistics::GetValuesByIndex(DWORD Index, TEMPREC& ret)
{
    if (m_dTempStat.empty() || (Index > (m_dTempStat.size() - 1)))
        return false;
    else {
        ret = m_dTempStatRaw[Index];
        return true;
    }
}


//---------------------------------------------------------------------------
bool CTempStatistics::GetFilteredValuesByIndex(DWORD Index, TEMPREC& ret)
{
    try {
        if (m_dTempStat.empty() || (Index > (m_dTempStat.size() - 1)))
            return false;
        else {
            ret = m_dTempStat[Index];
            return true;
        }
    } catch (...) {
        //sehr wahrscheinlich beim Zugriff auf m_dTempStat.size()
        //keine wirkliche Idee, warum das passiert
        return false;
    }
}


//---------------------------------------------------------------------------
void CTempStatistics::SetMaxIndexForMaximumCalculation(DWORD Index)
{
    if (Index > cMAX_GRAPHS - 1)
        return;

    m_dwMaxIndexForMaximumCalculation = Index;
    m_MaxTempSoFar                    = 0;
    m_MaxTempSoFarRaw                 = 0;
}


//---------------------------------------------------------------------------
void CTempStatistics::ResetMaxValues()
{
    m_MaxTempSoFar = m_MaxTempSoFarRaw = 0;

    size_t valuecount = m_dTempStatRaw.size();
    for (size_t i = 0; i < Min((int)m_dwMaxIndexForMaximumCalculation + 1, m_nStatWidth); i++) {
        for (size_t j = 0; j < valuecount; j++) {
            if (m_dTempStatRaw[j].T[i] > m_MaxTempSoFarRaw)
                m_MaxTempSoFarRaw = m_dTempStatRaw[j].T[i];
        }
    }

    valuecount = m_dTempStat.size();
    for (size_t i = 0; i < Min((int)m_dwMaxIndexForMaximumCalculation + 1, m_nStatWidth); i++) {
        for (size_t j = 0; j < valuecount; j++) {
            if (m_dTempStat[j].T[i] > m_MaxTempSoFar)
                m_MaxTempSoFar = m_dTempStat[j].T[i];
        }
    }
}
