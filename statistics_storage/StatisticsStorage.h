#ifndef STATISTICSSTORAGE_H
#define STATISTICSSTORAGE_H

#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>


class CStatisticsStorage {

    //    typedef time_t TDateTime;
    typedef std::chrono::system_clock::time_point TDateTime;

public:
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
     */
    CStatisticsStorage(size_t max_values_per_graph, size_t max_graphs,
                       size_t current_graphs, size_t filter_length, bool skip_zero_values_for_filtering)
        : m_current_value_count(0)
        , m_max_values(max_values_per_graph)
        , m_max_graphs(max_graphs)
        , m_current(0)
        , m_current_graph_count(current_graphs)
        , m_filter_length(filter_length)
        , m_ActiveCounter(0)
        , m_bActive(true)
        , m_bSkipZeroValuesForFiltering(skip_zero_values_for_filtering)
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

        TDateTime              timestamp = Now();
        std::vector<TDateTime> vts;
        vts.reserve(m_max_values);
        for (size_t j = 0; j < m_max_values; ++j) {
            m_timestamps.push_back(timestamp);
        }

        for (size_t i = 0; i < max_graphs; ++i) {
            m_data_raw.push_back(vval);
            m_data_filtered.push_back(vval);
        }

        m_max_value     = -1e10;
        m_max_value_raw = -1e10;
    }

    virtual ~CStatisticsStorage() {}

    size_t GetSize() const { return m_current_value_count; }

    size_t GetMaxValues() const { return m_max_values; }

    size_t GetMaxGraphs() const { return m_max_graphs; }


    void SetActive(bool active)
    {
        m_bActive = active;
        if (active)
            m_ActiveCounter = 0;
    }

    void SetSkipZeroValuesForFiltering(bool val = true) { m_bSkipZeroValuesForFiltering = val; }


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

        for (size_t i = 0; i < m_current_graph_count; ++i) {
            if (values[i] > m_max_value_raw) {
                m_max_value_raw = values[i];
            }
            m_data_raw[i][m_current] = values[i];
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
                    if (values[i] > m_max_value_raw) {
                        m_max_value_raw = values[i];
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
                if (values[i] > m_max_value_raw) {
                    m_max_value_raw = values[i];
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

    double GetMaxValue() const { return m_max_value; }

    double GetMaxValueRaw() const { return m_max_value_raw; }


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

private:
    void UpdateMaximumValues()
    {
        m_max_value = m_max_value_raw = -1e10;
        for (size_t i = 0; i < m_current_graph_count; ++i) {
            for (size_t j = 0; j < m_current_value_count; ++j) {
                if (m_data_raw[i][j] > m_max_value_raw) {
                    m_max_value_raw = m_data_raw[i][j];
                }
                if (m_data_filtered[i][j] > m_max_value) {
                    m_max_value = m_data_filtered[i][j];
                }
            }
        }
    }

    void AddCurrentRawValuesAsFilteredValues()
    {
        for (size_t i = 0; i < m_current_graph_count; ++i) {
            m_data_filtered[i][m_current] = m_data_raw[i][m_current];
            if (m_data_filtered[i][m_current] > m_max_value) {
                m_max_value = m_data_filtered[i][m_current];
            }
        }
    }


    // removes m_data_raw, m_data_filtered, m_keys and m_legend at given index
    // index has to exist (not checked here)
    // may be optimized with memmove/memcopy later on
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

        m_max_value = m_max_value_raw = -1e10;

        size_t filter_count = (m_current_value_count < m_filter_length) ? m_current_value_count : m_filter_length;

        for (size_t index = 0; index < m_current_value_count; ++index) {


            // bei gerade aktivierter GPU moeglichst steile 'An-Flanke'
            // -> Filterung nicht fuer m_nFilterLength sondern nur fuer die Anzahl der Werte nach der Aktivierung durchfuehren
            // m_bSkipZeroValuesForFiltering muss dafuer true sein
            if (m_ActiveCounter < filter_count) {
                filter_count = m_ActiveCounter;
            }


            if (filter_count == 0) {
                for (size_t i = 0; i < m_current_graph_count; ++i) {
                    m_data_filtered[i][index] = m_data_raw[i][index];
                    if (m_data_filtered[i][index] > m_max_value) {
                        m_max_value = m_data_filtered[i][index];
                    }
                }
            }

            for (size_t i = 0; i < m_current_graph_count; i++) {
                double fval        = 0;
                size_t sumCount    = 0;
                size_t filterIndex = index;

                for (size_t j = 0; j < filter_count; j++) {
                    if (!m_bSkipZeroValuesForFiltering) {
                        if (m_data_raw[i][filterIndex] != cINVALID_VALUE) {
                            fval += m_data_raw[i][filterIndex];
                            sumCount++;
                        }
                    } else {
                        if ((m_data_raw[i][filterIndex] != 0) && (m_data_raw[i][filterIndex] != cINVALID_VALUE)) {
                            fval += m_data_raw[i][filterIndex];
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
                m_data_filtered[i][index] = fval;

                // update maximum value
                if (fval > m_max_value) {
                    m_max_value = fval;
                }
            }
        }
    }

    void PerformFiltering()
    {
        if (m_current_value_count < 1)
            return;

        if (!m_filter_length) {
            AddCurrentRawValuesAsFilteredValues();
            return;
        }

        // bei deaktivierter GPU moeglichst steile 'Aus-Flanke'
        // -> keine Filterung
        if (!m_bActive) {
            AddCurrentRawValuesAsFilteredValues();
            return;
        }

        m_ActiveCounter++;

        size_t filter_count = (m_current_value_count < m_filter_length) ? m_current_value_count : m_filter_length;

        // bei gerade aktivierter GPU moeglichst steile 'An-Flanke'
        // -> Filterung nicht fuer m_nFilterLength sondern nur fuer die Anzahl der Werte nach der Aktivierung durchfuehren
        // m_bSkipZeroValuesForFiltering muss dafuer true sein
        if (m_ActiveCounter < filter_count) {
            filter_count = m_ActiveCounter;
        }

        if (filter_count == 0) {
            AddCurrentRawValuesAsFilteredValues();
            return;
        }

        for (size_t i = 0; i < m_current_graph_count; i++) {
            double fval        = 0;
            size_t sumCount    = 0;
            size_t filterIndex = m_current;

            for (size_t j = 0; j < filter_count; j++) {
                if (!m_bSkipZeroValuesForFiltering) {
                    if (m_data_raw[i][filterIndex] != cINVALID_VALUE) {
                        fval += m_data_raw[i][filterIndex];
                        sumCount++;
                    }
                } else {
                    if ((m_data_raw[i][filterIndex] != 0) && (m_data_raw[i][filterIndex] != cINVALID_VALUE)) {
                        fval += m_data_raw[i][filterIndex];
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
            m_data_filtered[i][m_current] = fval;

            // update maximum value
            if (fval > m_max_value) {
                m_max_value = fval;
            }
        }
    }


private:
    static std::chrono::system_clock::time_point Now()
    {
        return std::chrono::system_clock::now();
    }

    std::vector<std::vector<double> > m_data_raw;
    std::vector<std::vector<double> > m_data_filtered;
    std::vector<TDateTime>            m_timestamps;
    std::vector<std::wstring>         m_keys;
    std::vector<std::wstring>         m_legend;

    size_t m_current;             // pointer to current (most recently added) element
    size_t m_current_graph_count; // number of 'parallel' graphs
    size_t m_current_value_count; // number of current values in statistic
    size_t m_max_values;          // maximum number of values (reserved)
    size_t m_max_graphs;          // maximum number of 'parallel' graphs (reserved)
    size_t m_filter_length;       // filter length (if 0 -> filtering is disabled
    bool   m_bActive;
    size_t m_ActiveCounter;
    bool   m_bSkipZeroValuesForFiltering;
    bool   m_bUseValueAndKey; // state if statistics should use values and keys to determine indices or just indices

    double m_max_value;
    double m_max_value_raw;
};

const double CStatisticsStorage::cINVALID_VALUE = -1.23e40d;

#endif // STATISTICSSTORAGE_H
