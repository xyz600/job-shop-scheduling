#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>

struct Operation
{
    std::size_t operation_id;
    std::size_t process_time;
    std::size_t machine_id;
    std::size_t job_order;
    std::size_t job_id;

    nlohmann::json to_json() const
    {
        auto ret = nlohmann::json::object();
        ret["id"] = operation_id;
        ret["time"] = process_time;
        ret["machine"] = machine_id;
        ret["job"] = job_id;
        return ret;
    }
};

std::ostream &operator<<(std::ostream &out, const Operation &op)
{
    out << "(id, time, machine) = (" << op.operation_id << ", " << op.process_time << ", " << op.machine_id << ")";
    return out;
}

class Problem
{
public:
    std::size_t machine_size() const { return machine_size_; }
    std::size_t job_size() const { return job_size_; }

    std::size_t operation_size() const { return operation_list_.size(); }
    std::size_t job_length(const std::size_t job_id) const { return operation_table_[job_id].size(); }
    Operation &operation(std::size_t job_id, std::size_t order)
    {
        return operation_list_[operation_table_[job_id][order]];
    }
    const Operation &operation(std::size_t job_id, std::size_t order) const
    {
        return operation_list_[operation_table_[job_id][order]];
    }
    const Operation &operation(std::size_t operation_id) const { return operation_list_[operation_id]; }
    Operation &operation(std::size_t operation_id) { return operation_list_[operation_id]; }

    Problem(const std::string &filepath) { load(filepath); }

    void to_json(const std::string &filepath)
    {
        using json = nlohmann::json;
        json data;
        data["machine_size"] = machine_size();
        data["job_size"] = job_size();
        data["operation_table"] = {};
        for (auto &row : operation_table_)
        {
            auto ary = json::array();
            for (auto &elem : row)
            {
                ary.push_back(elem);
            }
            data["operation_table"].push_back(ary);
        }
        data["operations"] = {};
        for (auto &op : operation_list_)
        {
            data["operations"].push_back(op.to_json());
        }

        std::ofstream fout(filepath);
        if (!fout.is_open())
        {
            throw std::runtime_error("file cannot open @ Problem::to_json");
        }
        fout << data.dump(2) << std::endl;
    }

private:
    std::size_t machine_size_;
    std::size_t job_size_;
    std::vector<std::vector<std::size_t>> operation_table_;
    std::vector<Operation> operation_list_;

    void load(const std::string &filepath)
    {
        std::ifstream fin(filepath);
        if (!fin.is_open())
        {
            throw std::runtime_error("file not found @ Problem::load");
        }
        std::string _;
        {
            // skip header
            std::getline(fin, _);
            std::size_t time_seed, machine_seed, upper_bound, lower_bound;
            fin >> job_size_ >> machine_size_ >> time_seed >> machine_seed >> upper_bound >> lower_bound;
            // skip \n
            std::getline(fin, _);
        }
        operation_table_.resize(job_size_);

        // skip header
        std::getline(fin, _);
        // times
        {
            std::size_t id = 0;
            for (std::size_t job_id = 0; job_id < job_size_; job_id++)
            {
                auto &row = operation_table_[job_id];
                std::string line;
                std::getline(fin, line);

                boost::trim(line);
                std::vector<std::string> token;
                boost::split(token, line, boost::is_space(), boost::algorithm::token_compress_on);
                row.resize(token.size());

                for (std::size_t i = 0; i < token.size(); i++)
                {
                    row[i] = id++;
                    Operation op;
                    op.process_time = std::stol(token[i]);
                    op.operation_id = row[i];
                    op.job_id = job_id;
                    op.job_order = i;
                    operation_list_.push_back(op);
                }
            }
        }
        // skip header
        std::getline(fin, _);
        // machine
        {
            std::size_t operation_id = 0;
            for (auto &op : operation_list_)
            {
                fin >> op.machine_id;
            }
        }
    }
};

class Solution
{
public:
    struct State
    {
        std::size_t operation_id;
        std::size_t start_time;

        State(std::size_t operation_id, std::size_t start_time) : operation_id(operation_id), start_time(start_time) {}
        State() {}
        State(const State &src) : operation_id(src.operation_id), start_time(src.start_time) {}
    };

    Solution(const Problem &prob)
    {
        process_list_.resize(prob.machine_size() + 1);
        order_in_machine_.resize(prob.operation_size());
        std::fill(order_in_machine_.begin(), order_in_machine_.end(), std::numeric_limits<std::size_t>::max());
    }

    /**
     * @brief operation_id の直前の operation が登録されている前提で、machine および job
     * の制約を鑑みて最速で実行できる時間を計算
     */
    std::size_t fastest_start_time(const Problem &prob, const std::size_t operation_id) const
    {
        const auto machine_id = prob.operation(operation_id).machine_id;
        std::size_t time_by_machine = 0;
        if (!process_list_[machine_id].empty())
        {
            // current job の登録はまだ解になされていないため、push_back される前提
            const auto target_order = process_list_[machine_id].size() - 1;
            const auto target_operation_id = process_list_[machine_id][target_order].operation_id;
            time_by_machine =
                process_list_[machine_id][target_order].start_time + prob.operation(target_operation_id).process_time;
        }
        std::size_t time_by_job = 0;
        if (prob.operation(operation_id).job_order > 0)
        {
            const auto job_id = prob.operation(operation_id).job_id;
            const auto job_order = prob.operation(operation_id).job_order - 1;
            const auto &prev_op = prob.operation(job_id, job_order);
            time_by_job = process_list_[prev_op.machine_id][order_in_machine_[prev_op.operation_id]].start_time +
                          prev_op.process_time;
        }
        return std::max(time_by_machine, time_by_job);
    }

    void push_back_operation(const Problem &prob, const std::size_t operation_id)
    {
        const auto machine_id = prob.operation(operation_id).machine_id;
        order_in_machine_[operation_id] = process_list_[machine_id].size();
        // TODO: 修正
        process_list_[machine_id].emplace_back(operation_id, fastest_start_time(prob, operation_id));
    }

    void validate(const Problem &prob) const
    {
        std::vector<bool> used(prob.operation_size(), false);
        for (auto &row : process_list_)
        {
            for (auto &elem : row)
            {
                assert(elem.operation_id < prob.job_size() * prob.machine_size());
                used[elem.operation_id] = true;
            }
        }
        assert(std::count(used.begin(), used.end(), true) == prob.job_size() * prob.machine_size());

        // TODO: order_in_machine_ と process_list_ の整合性を確かめる
    }

    void to_json(const std::string &filepath)
    {
        using json = nlohmann::json;
        json data;
        data["process_list"] = {};
        for (std::size_t i = 1; i < process_list_.size(); i++)
        {
            auto &row = process_list_[i];
            auto ary = json::array();
            for (auto &elem : row)
            {
                auto obj = json::object();
                obj["start_time"] = elem.start_time;
                obj["operation_id"] = elem.operation_id;
                ary.push_back(obj);
            }
            data["process_list"].push_back(ary);
        }
        std::ofstream fout(filepath);
        if (!fout.is_open())
        {
            throw std::runtime_error("file cannot open @ Solution::to_json");
        }
        fout << data.dump(2) << std::endl;
    }

    void register_start_time(const Problem &prob)
    {
        std::vector<std::vector<std::size_t>> graph(prob.operation_size());
        std::vector<std::size_t> counter(prob.operation_size(), 0);

        // グラフの構築 job の依存関係
        for (std::size_t job_id = 0; job_id < prob.job_size(); job_id++)
        {
            for (std::size_t order = 0; order + 1 < prob.job_length(job_id); order++)
            {
                const auto cur_id = prob.operation(job_id, order).operation_id;
                const auto next_id = prob.operation(job_id, order + 1).operation_id;
                graph[cur_id].push_back(next_id);
                counter[next_id]++;
            }
        }

        // machine 処理順序の依存関係
        for (std::size_t machine_id = 1; machine_id <= prob.machine_size(); machine_id++)
        {
            for (std::size_t order = 0; order + 1 < process_list_[machine_id].size(); order++)
            {
                const auto cur_id = process_list_[machine_id][order].operation_id;
                const auto next_id = process_list_[machine_id][order + 1].operation_id;
                graph[cur_id].push_back(next_id);
                counter[next_id]++;
            }
        }

        // operation_id が sol のどこにあるかを構築
        std::vector<std::pair<std::size_t, std::size_t>> sol_place(prob.operation_size());
        for (std::size_t machine_id = 1; machine_id <= prob.machine_size(); machine_id++)
        {
            const auto &list = process_list_[machine_id];
            for (std::size_t order = 0; order < list.size(); order++)
            {
                const auto operation_id = process_list_[machine_id][order].operation_id;
                sol_place[operation_id].first = machine_id;
                sol_place[operation_id].second = order;
            }
        }

        // 初期化
        for (std::size_t machine_id = 1; machine_id <= prob.machine_size(); machine_id++)
        {
            for (auto &elem : process_list_[machine_id])
            {
                elem.start_time = 0;
            }
        }

        // topological sort order
        std::queue<std::size_t> que;
        for (std::size_t operation_id = 0; operation_id < prob.operation_size(); operation_id++)
        {
            if (counter[operation_id] == 0)
            {
                que.push(operation_id);
            }
        }

        while (!que.empty())
        {
            const auto operation_id = que.front();
            que.pop();

            const auto machine_id = sol_place[operation_id].first;
            const auto order_in_machine = sol_place[operation_id].second;
            const auto cur_time = process_list_[machine_id][order_in_machine].start_time;
            // 配る DP
            if (order_in_machine + 1 < process_list_[machine_id].size())
            {
                const auto next_start = cur_time + prob.operation(operation_id).process_time;
                process_list_[machine_id][order_in_machine + 1].start_time =
                    std::max(process_list_[machine_id][order_in_machine + 1].start_time, next_start);
            }

            const auto job_id = prob.operation(operation_id).job_id;
            const auto job_order = prob.operation(operation_id).job_order;
            if (job_order + 1 < prob.job_length(job_id))
            {
                const auto next_start = cur_time + prob.operation(operation_id).process_time;

                const auto &next_op = prob.operation(job_id, job_order + 1);
                const auto next_order_in_machine = sol_place[next_op.operation_id].second;
                process_list_[next_op.machine_id][next_order_in_machine].start_time =
                    std::max(process_list_[next_op.machine_id][next_order_in_machine].start_time, next_start);
            }

            for (const auto neighbor : graph[operation_id])
            {
                counter[neighbor]--;
                if (counter[neighbor] == 0)
                {
                    que.push(neighbor);
                }
            }
        }
    }

private:
    std::vector<std::vector<State>> process_list_;
    // operation id -> その operation が利用する機械で何番目に処理されるか
    std::vector<std::size_t> order_in_machine_;
};

class Solver
{
public:
    Solution solve(const Problem &prob)
    {
        Solution sol(prob);

        // 各 job の何番目の operation をこなしているか
        std::vector<std::size_t> operation_set(prob.job_size() + 1, 0);

        auto front_operation = [&](std::size_t job_id) { return prob.operation(job_id, operation_set[job_id]); };

        // 完了した job の個数
        std::size_t finish_counter = 0;

        while (finish_counter < prob.job_size())
        {
            // 終了時間が最短の命令を選択する
            std::size_t fastest_job_id = std::numeric_limits<std::size_t>::max();
            std::size_t fastest_finish_time = std::numeric_limits<std::size_t>::max();
            for (std::size_t job_id = 0; job_id < prob.job_size(); job_id++)
            {
                if (operation_set[job_id] < prob.job_length(job_id))
                {
                    const auto &operation = front_operation(job_id);
                    const std::size_t operation_id = operation.operation_id;

                    const auto finish_time = sol.fastest_start_time(prob, operation_id) + operation.process_time;
                    if (fastest_finish_time > finish_time)
                    {
                        fastest_finish_time = finish_time;
                        fastest_job_id = job_id;
                    }
                }
            }

            const auto &fastest_operation = front_operation(fastest_job_id);

            // random pick-up する operation の候補
            std::vector<std::size_t> conflict_set;
            for (std::size_t job_id = 0; job_id < prob.job_size(); job_id++)
            {
                if (operation_set[job_id] < prob.job_length(job_id))
                {
                    const auto &selected_operation = front_operation(job_id);
                    if (fastest_operation.machine_id == selected_operation.machine_id &&
                        sol.fastest_start_time(prob, selected_operation.operation_id) < fastest_finish_time)
                    {
                        conflict_set.push_back(job_id);
                    }
                }
            }

            // ランダムに1つ選択
            std::uniform_int_distribution<> rand(0, conflict_set.size() - 1);
            const auto selected_job_id = conflict_set[rand(mt)];
            sol.push_back_operation(prob, front_operation(selected_job_id).operation_id);

            // 同一 job を次に進める
            operation_set[selected_job_id]++;
            if (operation_set[selected_job_id] == prob.job_length(selected_job_id))
            {
                finish_counter++;
            }
        }
        return sol;
    }

private:
    std::mt19937_64 mt;
};

int main(int argc, char *argv[])
{
    const std::string filepath(argv[1]);
    Problem problem(filepath);

    std::cout << problem.job_size() << " " << problem.machine_size() << std::endl;
    std::cout << problem.operation(0, 0) << std::endl;

    Solver solver;
    auto sol = solver.solve(problem);
    sol.validate(problem);

    problem.to_json("problem.json");
    sol.to_json("answer.json");

    return 0;
}