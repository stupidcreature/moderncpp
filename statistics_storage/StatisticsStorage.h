#ifndef STATISTICSSTORAGE_H
#define STATISTICSSTORAGE_H

#include <algorithm>
//#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>
#include <string.h>

static const double cDEFAULT_MAX_RESET_VALUE = -1e10;

class CStatisticsStorage {

public:
#ifndef __BCPLUSPLUS__
    typedef time_t TDateTime;
#endif
    //typedef std::chrono::system_clock::time_point TDateTime;

    //    typedef struct {
    //        double                                value;
    //        std::chrono::system_clock::time_point timestamp;
    //    } DATA_VALUE_T;

    static const double cINVALID_VALUE;

    /** Constructor to create a StatisticsStorage
     * @param max_values_per_graph  how many values the statistics can hold
     * @param max_graphs            how many 'parallel' graphs can exist (for preallocating storage)
     * @param current_graphs        0 if key/legend scheme will be used, should be equal to max_graphs otherwise
     * @param filter_length         how many values should be used for filtering
     * @param skip_zero_values_for_filtering         should zero values in statistics be taken into account when filtering
     */
    CStatisticsStorage(size_t max_values_per_graph, size_t max_graphs,
                       size_t current_graphs, size_t filter_length, bool skip_zero_values_for_filtering)
        : m_current_value_count(0)
        , m_max_values(max_values_per_graph)
        , m_max_graphs(max_graphs)
        , m_current(0)
        , m_current_graph_count(current_graphs)
    {

        // if current_graphs == 0, then use keys to determine graph indices
        m_bUseValueAndKey = current_graphs == 0;

        m_data_raw.reserve(max_graphs);
        m_data_filtered.reserve(max_graphs);
        m_keys.reserve(max_graphs);
        m_legend.reserve(max_graphs);

        double              val = cINVALID_VALUE;
        std::vector<double> vval;
        vval.reserve(m_max_values);
        for (size_t j = 0; j < m_max_values; ++j) {
            vval.push_back(val);
        }
        m_output     = vval;
        m_output_raw = vval;

        TDateTime              timestamp = Now();
        std::vector<TDateTime> vts;
        vts.reserve(m_max_values);
        for (size_t j = 0; j < m_max_values; ++j) {
            m_timestamps.push_back(timestamp);
        }

        for (size_t i = 0; i < max_graphs; ++i) {
            m_data_raw.push_back(vval);
            m_data_filtered.push_back(vval);
            m_bActive.push_back(true);
            m_ActiveCounter.push_back(0);
            m_bSkipZeroValuesForFiltering.push_back(skip_zero_values_for_filtering);
            m_filter_length.push_back(filter_length);
            m_max_value.push_back(cDEFAULT_MAX_RESET_VALUE);
            m_max_value_raw.push_back(cDEFAULT_MAX_RESET_VALUE);
        }
    }

    virtual ~CStatisticsStorage() {}


    void Clear()
    {
        TDateTime now = Now();
        for (size_t i = 0; i < m_current_graph_count; ++i) {
            for (size_t j = 0; j < m_current_value_count; ++j) {
                m_data_raw[i][j]      = cINVALID_VALUE;
                m_data_filtered[i][j] = cINVALID_VALUE;
            }
            m_timestamps[i]    = now;
            m_ActiveCounter[i] = 0;
            m_bActive[i]       = true;
            m_max_value[i]     = cDEFAULT_MAX_RESET_VALUE;
            m_max_value_raw[i] = cDEFAULT_MAX_RESET_VALUE;
        }

        m_current             = 0;
        m_current_value_count = 0;

        // or keep them and only delete the values?
        // would be deleted anyway during next insert operation
        if (m_bUseValueAndKey) {
            m_keys.clear();
            m_legend.clear();
        }
    }

    int FindIndexForKey(const std::wstring& key)
    {
        for (size_t i = 0; i < m_current_graph_count; ++i) {
            if (key == m_keys[i]) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    size_t CopyInternalData(const std::vector<std::vector<double> >& vector, const int index, size_t num_values, double* target)
    {
        if (num_values > m_current_value_count) {
            num_values = m_current_value_count;
        }

        int start = m_current - num_values + 1;
        if (start < 0) {
            memcpy(target, &vector[index][start + m_max_values], sizeof(double) * -start);
            memcpy(target - start, &vector[index][0], sizeof(double) * (num_values + start));
        } else {
            memcpy(target, &vector[index][start], sizeof(double) * num_values);
        }
        return num_values;
    }

    size_t CopyInternalData(const std::vector<TDateTime>& vector, size_t num_values, double* target) const
    {
        if (num_values > m_current_value_count) {
            num_values = m_current_value_count;
        }

        int start = m_current - num_values + 1;
        if (start < 0) {
            memcpy(target, &vector[start + m_max_values], sizeof(double) * -start);
            memcpy(target - start, &vector[0], sizeof(double) * (num_values + start));
        } else {
            memcpy(target, &vector[start], sizeof(double) * num_values);
        }
        return num_values;
    }

    size_t GetLastTimestamps(size_t num_values, double* values_out) const
    {
        return CopyInternalData(m_timestamps, num_values, values_out);
    }

    size_t GetLastValuesForIndex(const size_t index, size_t num_values, double* values_out)
    {
        if (index >= m_current_graph_count) {
            return 0;
        }

        return CopyInternalData(m_data_filtered, index, num_values, values_out);
    }

    size_t GetLastValuesForIndexAsCopy(const size_t index, size_t num_values, double*& values_out)
    {
        if (index >= m_current_graph_count) {
            return 0;
        }

        size_t ret = CopyInternalData(m_data_filtered, index, num_values, &m_output[0]);
        values_out = &m_output[0];
        return ret;
    }

    size_t GetLastRawValuesForIndexAsCopy(const size_t index, size_t num_values, double*& values_out)
    {
        if (index >= m_current_graph_count) {
            return 0;
        }

        size_t ret = CopyInternalData(m_data_raw, index, num_values, &m_output_raw[0]);
        values_out = &m_output_raw[0];
        return ret;
    }


    size_t GetLastRawValuesForIndex(const size_t index, size_t num_values, double* values_out)
    {
        if (index >= m_current_graph_count) {
            return 0;
        }

        return CopyInternalData(m_data_raw, index, num_values, values_out);
    }

    size_t GetLastValuesForKey(const std::wstring& key, size_t num_values, double* values_out)
    {
        int index = FindIndexForKey(key);
        if (index < 0) {
            return 0;
        }

        return CopyInternalData(m_data_filtered, index, num_values, values_out);
    }


    size_t GetLastRawValuesForKey(const std::wstring& key, size_t num_values, double* values_out)
    {
        int index = FindIndexForKey(key);
        if (index < 0) {
            return 0;
        }

        return CopyInternalData(m_data_raw, index, num_values, values_out);
    }


    size_t GetLastTimestamps(size_t num_values, TDateTime* values_out)
    {
        if (num_values > m_current_value_count) {
            num_values = m_current_value_count;
        }

        int start = m_current - num_values + 1;
        if (start < 0) {
            memcpy(values_out, &m_timestamps[start + m_max_values], sizeof(TDateTime) * -start);
            memcpy(values_out - start, &m_timestamps[0], sizeof(TDateTime) * (num_values + start));
        } else {
            memcpy(values_out, &m_timestamps[start], sizeof(TDateTime) * num_values);
        }
        return num_values;
    }


    size_t GetSize() const { return m_current_value_count; }

    size_t GetMaxValues() const { return m_max_values; }

    size_t GetMaxGraphs() const { return m_max_graphs; }

    size_t GetKeyCount() const { return m_keys.size(); }

    void ResetMaxValues() {
        RefilterAll();
    }


    void SetActive(size_t index, bool active)
    {
        if (index >= m_current_graph_count) {
            return;
        }

        m_bActive[index] = active;
        if (active)
            m_ActiveCounter[index] = 0;
    }

    void SetSkipZeroValuesForFiltering(size_t index, bool val = true)
    {
        if (index >= m_current_graph_count) {
            return;
        }
        m_bSkipZeroValuesForFiltering[index] = val;
    }


    // adds a vector of values and applies the timestamp 'Now()'
    size_t AddValues(std::vector<double> values)
    {
        if (m_bUseValueAndKey) {
            throw(std::runtime_error("AddValues: statistics intended for use with keys -> otherwise set number of graphs manually."));
        }

        if (m_current_graph_count != values.size()) {
            // how to react to a wrong number of elements passed to the
            // function?
            throw(std::runtime_error("AddValues: wrong number of values"));
        }

        //determine the position where to insert new value
        if (m_current_value_count) {
            m_current = (m_current + 1) % m_max_values;
        }

        for (size_t graph_index = 0; graph_index < m_current_graph_count; ++graph_index) {
            if (values[graph_index] > m_max_value_raw[graph_index]) {
                m_max_value_raw[graph_index] = values[graph_index];
            }
            m_data_raw[graph_index][m_current] = values[graph_index];
        }
        m_timestamps[m_current] = Now();

        if (m_current_value_count < m_max_values) {
            ++m_current_value_count;
        }

        PerformFiltering();

        return m_current_value_count;
    }


    // adds a vector of values, inserts them according to the keys provided and applies the timestamp 'Now()'
    // clang-format off
    // strategy:
    // add values with the following keys [a, b, c, f] to a statistics with existing keys [a, c, d, e, f]
    // 1) first, check for existing keys to be able to remove all non-existing ones
    // -> d + e don't exist in new values
    //
    // 2) remove/reorder values + legend entries for which no new keys exist
    // -> [a, c, f]
    // 2b) len = length([a,c,f]) (== 3)
    //
    // 3) if step 2 was required (keys had to be removed) then redo step 1 to mark positions which
    //    key has to be inserted into which position
    // -> [a, b,  c, f]
    //     0 len  1  2    (every time a non-existing value is found, insert at position len and increment len)
    //
    // don't forget to:
    //     add legend values
    //     add new keys at the end of to m_keys
    //     adjust m_current_graph_count
    //
    // clang-format on
    size_t AddValuesWithKeys(std::vector<double> values, std::vector<std::wstring> keys, std::vector<std::wstring> legend)
    {
        if (!m_bUseValueAndKey) {
            throw(std::runtime_error("AddValuesWithKeys: statistics not intended for use with keys -> set current_graphs to 0 when calling ctor."));
        }
        if (values.size() > m_max_graphs) {
            throw(std::runtime_error("AddValuesWithKeys: too many values"));
        }
        if (values.size() != keys.size()) {
            throw(std::runtime_error("AddValuesWithKeys: values and keys have to have the same length"));
        }
        if (values.size() != legend.size()) {
            throw(std::runtime_error("AddValuesWithKeys: values, keys and legend have to have the same length"));
        }


        bool bToRemove            = true;
        bool bUpdateMaximumValues = false;

        while (bToRemove) {
            std::vector<int> to_remove(m_keys.size(), -1);
            size_t           iFoundCount = 0;
            for (size_t i = 0; i < m_keys.size(); ++i) {
                for (size_t j = 0; j < keys.size(); ++j) {
                    if (m_keys[i] == keys[j]) {
                        to_remove[i] = (int)j;
                        ++iFoundCount;
                    }
                }
            }

            // all keys found -> no values to remove
            if (iFoundCount == to_remove.size()) {
                bToRemove = false;
            } else {
                for (size_t i = 0; i < to_remove.size(); ++i) {
                    // no new values for key == i
                    // -> remove values for this key and reorder remaining values, keys and legend text
                    if (-1 == to_remove[i]) {
                        RemoveFromStatistics(i);
                        bUpdateMaximumValues = true;
                        break;
                    }
                }
            }
        }

        // determine the position where to insert new value
        if (m_current_value_count) {
            m_current = (m_current + 1) % m_max_values;
        }


        for (size_t i = 0; i < keys.size(); ++i) {
            // check, if keys[i] exists in m_keys
            bool bFound = false;
            for (size_t j = 0; j < m_keys.size(); ++j) {
                if (keys[i] == m_keys[j]) {
                    // yes, found at position j -> insert value, key, legend
                    if (values[i] > m_max_value_raw[i]) {
                        m_max_value_raw[i] = values[i];
                    }
                    m_data_raw[j][m_current] = values[i];
                    m_keys[j]                = keys[i];
                    m_legend[j]              = legend[i];
                    bFound                   = true;
                    break;
                }
            }

            if (!bFound && m_keys.size() < m_max_graphs) {
                // key with value keys[i] not found -> append to statistics if current_keys < m_max_graphs
                if (values[i] > m_max_value_raw[i]) {
                    m_max_value_raw[i] = values[i];
                }
                m_data_raw[m_keys.size()][m_current] = values[i];
                m_keys.push_back(keys[i]);
                m_legend.push_back(legend[i]);
                ++m_current_graph_count;
            }
        }

        m_timestamps[m_current] = Now();


        if (m_current_value_count < m_max_values) {
            ++m_current_value_count;
        }

        PerformFiltering();

        if (bUpdateMaximumValues) {
            UpdateMaximumValues();
        }

        return m_current_value_count;
    }

    std::vector<std::wstring> GetLegendVector() const
    {
        if (m_bUseValueAndKey)
            return m_legend;

        return std::vector<std::wstring>(1, L"NO LEGEND VALUES");
    }

    std::vector<std::wstring> GetKeyVector() const
    {
        if (m_bUseValueAndKey)
            return m_keys;

        return std::vector<std::wstring>(1, L"NO KEYS");
    }

    std::wstring GetKeyForIndex(size_t index) const
    {
        if (m_bUseValueAndKey && (index < m_current_graph_count))
            return m_keys[index];
        return L"";
    }

    double GetMaxValue(size_t index) const
    {
        if (index >= m_current_graph_count) {
            return cDEFAULT_MAX_RESET_VALUE;
        }
        return m_max_value[index];
    }

    double GetMaxValueRaw(size_t index) const
    {
        if (index >= m_current_graph_count) {
            return cDEFAULT_MAX_RESET_VALUE;
        }
        return m_max_value_raw[index];
    }


    std::vector<double> GetValuesForKey(const std::wstring& key) const
    {
        for (size_t i = 0; i < m_current_graph_count; ++i) {
            if (key == m_keys[i]) {
                return std::vector<double>(m_data_filtered[i]);
            }
        }
        return std::vector<double>();
    }


    std::vector<double> GetRawValuesForKey(const std::wstring& key) const
    {
        for (size_t i = 0; i < m_current_graph_count; ++i) {
            if (key == m_keys[i]) {
                return std::vector<double>(m_data_raw[i]);
            }
        }
        return std::vector<double>();
    }

    std::vector<TDateTime> GetTimestamps() const
    {
        return std::vector<TDateTime>(m_timestamps);
    }


    void UpdateFilterLengthAll(size_t filter_length)
    {
        for (size_t i = 0; i < m_current_graph_count; ++i) {
            m_filter_length[i] = filter_length;
        }
        RefilterAll();
    }

    void UpdateFilterLengthByIndex(size_t index, size_t filter_length)
    {
        if (index >= m_current_graph_count) {
            return;
        }

        m_filter_length[index] = filter_length;

        RefilterAll(); //todo: refilter only one graph
    }

private:
    void UpdateMaximumValues()
    {
        for (size_t graph_index = 0; graph_index < m_current_graph_count; ++graph_index) {
            m_max_value[graph_index] = m_max_value_raw[graph_index] = cDEFAULT_MAX_RESET_VALUE;
            for (size_t value_index = 0; value_index < m_current_value_count; ++value_index) {
                if (m_data_raw[graph_index][value_index] > m_max_value_raw[graph_index]) {
                    m_max_value_raw[graph_index] = m_data_raw[graph_index][value_index];
                }
                if (m_data_filtered[graph_index][value_index] > m_max_value[graph_index]) {
                    m_max_value[graph_index] = m_data_filtered[graph_index][value_index];
                }
            }
        }
    }

    void AddCurrentRawValuesAsFilteredValues()
    {
        for (size_t graph_index = 0; graph_index < m_current_graph_count; ++graph_index) {
            m_data_filtered[graph_index][m_current] = m_data_raw[graph_index][m_current];
            if (m_data_filtered[graph_index][m_current] > m_max_value[graph_index]) {
                m_max_value[graph_index] = m_data_filtered[graph_index][m_current];
            }
        }
    }


    void AddCurrentRawValueAsFilteredValue(size_t index)
    {
        m_data_filtered[index][m_current] = m_data_raw[index][m_current];
        if (m_data_filtered[index][m_current] > m_max_value[index]) {
            m_max_value[index] = m_data_filtered[index][m_current];
        }
    }

    // removes m_data_raw, m_data_filtered, m_keys and m_legend at given index
    // index has to exist (not checked here)
    // may be optimized with memmove/memcpy later on
    // but not important, as this functions should rarely be called
    void RemoveFromStatistics(size_t index_to_remove)
    {
        if (!m_current_graph_count) {
            return;
        }


        for (size_t i = index_to_remove; i < m_current_graph_count - 2; ++i) {
            for (size_t j = 0; j < m_current_value_count; ++j) {
                m_data_raw[i][j]      = m_data_raw[i + 1][j];
                m_data_filtered[i][j] = m_data_filtered[i + 1][j];
            }
            m_keys[i]   = m_keys[i + 1];
            m_legend[i] = m_legend[i + 1];
        }

        double val = cINVALID_VALUE;
        for (size_t j = 0; j < m_current_value_count; ++j) {
            m_data_raw[m_current_graph_count - 1][j]      = val;
            m_data_filtered[m_current_graph_count - 1][j] = val;
        }
        m_keys.pop_back();
        m_legend.pop_back();
        --m_current_graph_count;

        // reset the timestamp vector after deleting the last graph
        if (!m_current_graph_count) {
            TDateTime now = Now();
            for (size_t j = 0; j < m_current_value_count; ++j) {
                m_timestamps[j] = now;
            }
        }
    }


    void RefilterAll()
    {
        for (size_t k = 0; k < m_current_graph_count; ++k) {
            m_max_value[k] = m_max_value_raw[k] = cDEFAULT_MAX_RESET_VALUE;
        }

        for (size_t value_index = 0; value_index < m_current_value_count; ++value_index) {

            for (size_t graph_index = 0; graph_index < m_current_graph_count; ++graph_index) {

                if (!m_filter_length[graph_index]) {
                    AddCurrentRawValueAsFilteredValue(graph_index);
                    continue;
                }

                // bei deaktivierter GPU moeglichst steile 'Aus-Flanke'
                // -> keine Filterung
                if (!m_bActive[graph_index]) {
                    AddCurrentRawValueAsFilteredValue(graph_index);
                    continue;
                }

                ++m_ActiveCounter[graph_index];

                size_t filter_count = (m_current_value_count < m_filter_length[graph_index]) ? m_current_value_count : m_filter_length[graph_index];

                // bei gerade aktivierter GPU moeglichst steile 'An-Flanke'
                // -> Filterung nicht fuer m_nFilterLength sondern nur fuer die Anzahl der Werte nach der Aktivierung durchfuehren
                // m_bSkipZeroValuesForFiltering muss dafuer true sein
                if (m_ActiveCounter[graph_index] < filter_count) {
                    filter_count = m_ActiveCounter[graph_index];
                }

                if (filter_count == 0) {
                    AddCurrentRawValueAsFilteredValue(graph_index);
                    continue;
                }

                double fval        = 0;
                size_t sumCount    = 0;
                size_t filterIndex = value_index;

                for (size_t filter_index = 0; filter_index < filter_count; ++filter_index) {
                    if (!m_bSkipZeroValuesForFiltering[graph_index]) {
                        if (m_data_raw[graph_index][filterIndex] != cINVALID_VALUE) {
                            fval += m_data_raw[graph_index][filterIndex];
                            sumCount++;
                        }
                    } else {
                        if ((m_data_raw[graph_index][filterIndex] != 0) && (m_data_raw[graph_index][filterIndex] != cINVALID_VALUE)) {
                            fval += m_data_raw[graph_index][filterIndex];
                            sumCount++;
                        }
                    }
                    if (filterIndex > 0) {
                        --filterIndex;
                    } else {
                        filterIndex = m_max_values - 1;
                    }
                }
                if (sumCount > 0) {
                    fval /= sumCount;
                } else {
                    fval = cINVALID_VALUE;
                }
                m_data_filtered[graph_index][m_current] = fval;

                // update maximum value
                if (fval > m_max_value[graph_index]) {
                    m_max_value[graph_index] = fval;
                }
            }
        }
    }

    void PerformFiltering()
    {
        if (m_current_value_count < 1)
            return;

        for (size_t graph_index = 0; graph_index < m_current_graph_count; ++graph_index) {

            if (!m_filter_length[graph_index]) {
                AddCurrentRawValueAsFilteredValue(graph_index);
                continue;
            }

            // bei deaktivierter GPU moeglichst steile 'Aus-Flanke'
            // -> keine Filterung
            if (!m_bActive[graph_index]) {
                AddCurrentRawValueAsFilteredValue(graph_index);
                continue;
            }

            ++m_ActiveCounter[graph_index];

            size_t filter_count = (m_current_value_count < m_filter_length[graph_index]) ? m_current_value_count : m_filter_length[graph_index];

            // bei gerade aktivierter GPU moeglichst steile 'An-Flanke'
            // -> Filterung nicht fuer m_nFilterLength sondern nur fuer die Anzahl der Werte nach der Aktivierung durchfuehren
            // m_bSkipZeroValuesForFiltering muss dafuer true sein
            if (m_ActiveCounter[graph_index] < filter_count) {
                filter_count = m_ActiveCounter[graph_index];
            }

            if (filter_count == 0) {
                AddCurrentRawValueAsFilteredValue(graph_index);
                continue;
            }

            double fval        = 0;
            size_t sumCount    = 0;
            size_t filterIndex = m_current;

            for (size_t filter_index = 0; filter_index < filter_count; ++filter_index) {
                if (!m_bSkipZeroValuesForFiltering[graph_index]) {
                    if (m_data_raw[graph_index][filterIndex] != cINVALID_VALUE) {
                        fval += m_data_raw[graph_index][filterIndex];
                        sumCount++;
                    }
                } else {
                    if ((m_data_raw[graph_index][filterIndex] != 0) && (m_data_raw[graph_index][filterIndex] != cINVALID_VALUE)) {
                        fval += m_data_raw[graph_index][filterIndex];
                        sumCount++;
                    }
                }
                if (filterIndex > 0) {
                    --filterIndex;
                } else {
                    filterIndex = m_max_values - 1;
                }
            }
            if (sumCount > 0) {
                fval /= sumCount;
            } else {
                fval = cINVALID_VALUE;
            }
            m_data_filtered[graph_index][m_current] = fval;

            // update maximum value
            if (fval > m_max_value[graph_index]) {
                m_max_value[graph_index] = fval;
            }
        }
    }


private:
#ifndef __BCPLUSPLUS__
    static TDateTime Now()
    {
        static TDateTime now = -1;
        now += 1;
        return now;
        //        return std::chrono::system_clock::now();
    }
#endif

    std::vector<std::vector<double> > m_data_raw;
    std::vector<std::vector<double> > m_data_filtered;
    std::vector<TDateTime>            m_timestamps;
    std::vector<std::wstring>         m_keys;
    std::vector<std::wstring>         m_legend;

    std::vector<double> m_output;       // vector with copy of internal data to be handed out
    std::vector<double> m_output_raw;   // vector with copy of internal raw data to be handed out

    std::vector<bool>   m_bActive;
    std::vector<size_t> m_ActiveCounter;
    std::vector<bool>   m_bSkipZeroValuesForFiltering;
    std::vector<size_t> m_filter_length; // filter length (set to 0 to disable filtering)

    std::vector<double> m_max_value;
    std::vector<double> m_max_value_raw;

    size_t m_current;             // pointer to current (most recently added) element
    size_t m_current_graph_count; // number of 'parallel' graphs
    size_t m_current_value_count; // number of current values in statistic
    size_t m_max_values;          // maximum number of values (reserved)
    size_t m_max_graphs;          // maximum number of 'parallel' graphs (reserved)
    bool   m_bUseValueAndKey;     // state if statistics should use values and keys to determine indices or just indices
};


#ifndef __BCPLUSPLUS__
const double CStatisticsStorage::cINVALID_VALUE = -1.23e40d;
#else
const double CStatisticsStorage::cINVALID_VALUE = -1.23e40;
#endif


#endif // STATISTICSSTORAGE_H
