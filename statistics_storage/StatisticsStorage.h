#ifndef STATISTICSSTORAGE_H
#define STATISTICSSTORAGE_H

#include <vector>

class CStatisticsStorage {
public:
    CStatisticsStorage(std::size_t max_values_per_graph, std::size_t max_graphs) {
        m_data_raw.reserve(max_graphs);
        m_data_filtered.reserve(max_graphs);

        for (int i = 0; i < max_values_per_graph; ++i) {
            m_data_raw[i].reserve(max_values_per_graph);
            m_data_filtered[i].reserve(max_values_per_graph);
        }

        m_start = 0;
        m_current = 0;
    }


    virtual ~CStatisticsStorage() {}


private:
    CStatisticsStorage() {}

    std::vector<std::vector<double>> m_data_raw;
    std::vector<std::vector<double>> m_data_filtered;

    std::size_t m_start;
    std::size_t m_current;
};

#endif //STATISTICSSTORAGE_H
