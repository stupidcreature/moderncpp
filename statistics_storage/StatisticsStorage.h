#ifndef STATISTICSSTORAGE_H
#define STATISTICSSTORAGE_H

#include <chrono>
#include <vector>
#include <stdexcept>

class CStatisticsStorage {

    typedef struct {
        double value;
        std::chrono::system_clock::time_point timestamp;
    } DATA_VALUE_T;

public:
    CStatisticsStorage(size_t max_values_per_graph, size_t max_graphs,
                       size_t current_graphs, size_t filter_length)
            : m_current_values(0), m_max_values(max_values_per_graph),
              m_max_graphs(max_graphs), m_start(0), m_current(0),
              m_current_graphs(current_graphs), m_filter_length(filter_length) {

        m_data_raw.reserve(max_graphs);
        m_data_filtered.reserve(max_graphs);

        DATA_VALUE_T val;
        val.value = 0;
        val.timestamp = Now();


        std::vector<DATA_VALUE_T> vval;
        for (int j = 0; j < m_max_values; ++j) {
            vval.push_back(val);
        }

        for (int i = 0; i < max_graphs; ++i) {
            m_data_raw[i].reserve(max_values_per_graph);
            m_data_filtered[i].reserve(max_values_per_graph);

            m_data_raw.push_back(vval);
            m_data_filtered.push_back(vval);
        }

    }

    virtual ~CStatisticsStorage() {}

    size_t GetSize() const { return m_current_values; }

    size_t GetMaxValues() const { return m_max_values; }

    size_t GetMaxGraphs() const { return m_max_graphs; }

    // adds a vector of values and applies the timestamp 'Now()'
    size_t AddValues(std::vector<double> values) {
        if (m_current_graphs != values.size()) {
            // how to react to a wrong number of elements passed to the
            // function?
            throw (std::runtime_error("AddValues: wrong number of values"));
        }

        // if buffer is not filled completely, just add a new value
        if (m_current_values < m_max_values) {
            DATA_VALUE_T val;
            val.timestamp = Now();
            for (int i = 0; i < m_current_graphs; ++i) {
                val.value = values[i];
                m_data_raw[i].push_back(val);
            }
        }
            // buffer is already full -> overwrite oldest value
        else {
        }
    }

private:
    CStatisticsStorage() {}

    static std::chrono::system_clock::time_point Now() {
        return std::chrono::system_clock::now();
    }

    std::vector<std::vector<DATA_VALUE_T>> m_data_raw;
    std::vector<std::vector<DATA_VALUE_T>> m_data_filtered;

    size_t m_start;          // pointer to first element
    size_t m_current;        // pointer to current (most recently added) element
    size_t m_current_graphs; // number of 'parallel' graphs
    size_t m_current_values; // number of current values in statistic
    size_t m_max_values;     // maximum number of values (reserved)
    size_t m_max_graphs;     // maximum number of 'parallel' graphs (reserved)
    size_t m_filter_length;  // filter length (if 0 -> filtering is disabled
};

#endif // STATISTICSSTORAGE_H
