#ifndef STATISTICSSTORAGE_H
#define STATISTICSSTORAGE_H

#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>

class CStatisticsStorage {

public:
    typedef struct {
        double                                value;
        std::chrono::system_clock::time_point timestamp;
    } DATA_VALUE_T;


    CStatisticsStorage(size_t max_values_per_graph, size_t max_graphs,
                       size_t current_graphs, size_t filter_length)
        : m_current_value_count(0)
        , m_max_values(max_values_per_graph)
        , m_max_graphs(max_graphs)
        , m_current(0)
        , m_current_graph_count(current_graphs)
        , m_filter_length(filter_length)
        , m_ActiveCounter(0)
        , m_bActive(true)
        , m_bSkipZeroValuesForFiltering(false)
    {

        m_data_raw.reserve(max_graphs);
        m_data_filtered.reserve(max_graphs);
        m_keys.reserve(max_graphs);
        m_legend.reserve(max_graphs);

        DATA_VALUE_T val;
        val.value     = 0;
        val.timestamp = Now();


        std::vector<DATA_VALUE_T> vval;
        vval.reserve(m_max_values);
        for (size_t j = 0; j < m_max_values; ++j) {
            vval.push_back(val);
        }

        for (size_t i = 0; i < max_graphs; ++i) {
            m_data_raw.push_back(vval);
            m_data_filtered.push_back(vval);
            m_keys.push_back(L"");
            m_legend.push_back(L"");
        }
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
        if (m_current_graph_count != values.size()) {
            // how to react to a wrong number of elements passed to the
            // function?
            throw(std::runtime_error("AddValues: wrong number of values"));
        }

        //determine the position where to insert new value
        if (m_current_value_count) {
            m_current = (m_current + 1) % m_max_values;
        }

        DATA_VALUE_T val;
        val.timestamp = Now();
        for (size_t i = 0; i < m_current_graph_count; ++i) {
            val.value                = values[i];
            m_data_raw[i][m_current] = val;
        }

        if (m_current_value_count < m_max_values) {
            ++m_current_value_count;
        }

        PerformFiltering();

        return m_current_value_count;
    }


    // adds a vector of values, inserts them according to the keys provided and applies the timestamp 'Now()'
    size_t AddValuesWithKeys(std::vector<double> values, std::vector<std::wstring> keys, std::vector<std::wstring> legend)
    {
        if (m_current_graph_count != values.size()) {
            // how to react to a wrong number of elements passed to the
            // function?
            throw(std::runtime_error("AddValuesWithKeys: wrong number of values"));
        }
        if (m_current_graph_count != keys.size()) {
            throw(std::runtime_error("AddValuesWithKeys: values and keys have to have the same length"));
        }
        if (m_current_graph_count != legend.size()) {
            throw(std::runtime_error("AddValuesWithKeys: values, keys and legend have to have the same length"));
        }

        //determine the position where to insert new value
        if (m_current_value_count) {
            m_current = (m_current + 1) % m_max_values;
        }

        DATA_VALUE_T val;
        val.timestamp = Now();
        for (size_t i = 0; i < m_current_graph_count; ++i) {
            val.value                = values[i];
            m_data_raw[i][m_current] = val;
        }

        if (m_current_value_count < m_max_values) {
            ++m_current_value_count;
        }

        PerformFiltering();

        return m_current_value_count;
    }


private:
    void AddCurrentRawValuesAsFilteredValues()
    {
        for (size_t i = 0; i < m_current_graph_count; i++) {
            m_data_filtered[i][m_current] = m_data_raw[i][m_current];
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

        if (filter_count == 0) {
            AddCurrentRawValuesAsFilteredValues();
            return;
        }

        // bei gerade aktivierter GPU moeglichst steile 'An-Flanke'
        // -> Filterung nicht fuer m_nFilterLength sondern nur fuer die Anzahl der Werte nach der Aktivierung durchfuehren
        // m_bSkipZeroValuesForFiltering muss dafuer true sein
        if (m_ActiveCounter < filter_count) {
            filter_count = m_ActiveCounter;
        }


        for (size_t i = 0; i < m_current_graph_count; i++) {
            double fval        = 0;
            size_t sumCount    = 0;
            size_t filterIndex = m_current;

            DATA_VALUE_T val = m_data_raw[i][m_current];

            for (size_t j = 0; j < filter_count; j++) {
                if (!m_bSkipZeroValuesForFiltering) {
                    fval += m_data_raw[i][filterIndex].value;
                    sumCount++;
                } else {
                    if (m_data_raw[i][filterIndex].value > 0) {
                        fval += m_data_raw[i][filterIndex].value;
                        sumCount++;
                    }
                }
                if (filterIndex > 0) {
                    --filterIndex;
                } else {
                    filterIndex = m_max_values - 1;
                }
            }
            if (sumCount > 0)
                fval /= sumCount;
            val.value                     = fval;
            m_data_filtered[i][m_current] = val;
        }
    }


private:
    static std::chrono::system_clock::time_point Now()
    {
        return std::chrono::system_clock::now();
    }

    std::vector<std::vector<DATA_VALUE_T> > m_data_raw;
    std::vector<std::vector<DATA_VALUE_T> > m_data_filtered;
    std::vector<std::wstring>               m_keys;
    std::vector<std::wstring>               m_legend;

    size_t m_current;             // pointer to current (most recently added) element
    size_t m_current_graph_count; // number of 'parallel' graphs
    size_t m_current_value_count; // number of current values in statistic
    size_t m_max_values;          // maximum number of values (reserved)
    size_t m_max_graphs;          // maximum number of 'parallel' graphs (reserved)
    size_t m_filter_length;       // filter length (if 0 -> filtering is disabled
    bool   m_bActive;
    size_t m_ActiveCounter;
    bool   m_bSkipZeroValuesForFiltering;
};

#endif // STATISTICSSTORAGE_H
