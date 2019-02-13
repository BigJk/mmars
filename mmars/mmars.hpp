#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <unordered_map>

#include "warrior.hpp"

/**
 * \brief Represents a fighting result
 */
class result
{
public:
    uint16_t win = 0;
    uint16_t loss = 0;
    uint16_t tie = 0;

    result(uint16_t win, uint16_t loss, uint16_t tie)
        : win(win),
          loss(loss),
          tie(tie)
    {
    }

    result() = default;
};

/**
 * \brief The mars implementation
 */
class mmars
{
private:
    int                                                     _seed = -1;
    uint16_t                                                _round = 0;
    std::vector<std::shared_ptr<warrior>>                   _warriors;
    std::unordered_map<std::shared_ptr<warrior>, result>    _results;
    std::vector<std::queue<uint32_t>>                       _task_queue;
    std::vector<instruction>                                _core;


    /**
     * \brief Folds a pointer to stay inside the core size and read / write range.
     * \param ptr Pointer inside the core
     * \param limit Limit to fold to
     * \return The folded pointer
     */
    inline uint32_t fold(uint32_t ptr, uint32_t limit) const;

    /**
     * \brief Enqueue a task into the queue of a warrior.
     * \param wi Warrior index
     * \param ptr Task pointer
     */
    inline void queue(int wi, uint32_t ptr);

    /**
     * \brief Implements a minimal random number generator. Source is borrowed from pmars to be able to reproduce deterministic results equal to pmars or exmars.
     * \return Next random number
     */
    int random();


    /**
     * \brief Inserts a warrior into the core at the given position.
     * \param wi Warrior index
     * \param p Target position
     * \return Target position + warrior size
     */
    uint32_t copy_warrior(int wi, uint32_t p);

    /**
     * \brief Inserts all warriors into the core.
     */
    void insert_warriors();

public:
    uint32_t core_size      = 8000;
    uint32_t max_cycles     = 80000;
    uint32_t max_process    = 8000;
    uint32_t max_length     = 200;
    uint32_t min_separation = 200;

    uint32_t read_limit     = 8000;
    uint32_t write_limit    = 8000;

    mmars(uint32_t core_size, uint32_t max_cycles, uint32_t max_process, uint32_t max_length, uint32_t min_separation)
        : core_size(core_size),
          max_cycles(max_cycles),
          max_process(max_process),
          max_length(max_length),
          min_separation(min_separation)
    {
        read_limit = core_size;
        write_limit = core_size;
    }

    mmars() = default;

    /**
     * \brief Clears everything. This includes warriors.
     */
    void clear();

    /**
     * \brief Adds a warrior to the mars.
     */
    void add_warrior(std::shared_ptr<warrior> w);

    /**
     * \brief Returns the fight results of a warrior
     * \return Results
     */
    result get_result(std::shared_ptr<warrior> w);

    /**
     * \brief Sets the seed of the random number generator.
     * \param seed The seed
     */
    void set_seed(uint32_t seed);

    /**
     * \brief Resets mars to a pre-fight state. This should be called before a new round.
     */
    void setup();

    /**
     * \brief Executes one cycle of each warrior.
     * \return Count of alive warriors
     */
    uint32_t step();

    /**
     * \brief Runs a fight over multiple rounds.
     * \param rounds Amount of rounds
     */
    void run(int rounds);

    /**
     * \brief Gets a instruction from the core.
     * \param i Index of the instruction
     * \return The instruction
     */
    instruction get_instruction(uint32_t i);

    /**
     * \brief Gets the task queue of a warrior.
     * \param w The warrior
     * \return The task queue as vector
     */
    std::vector<uint32_t> get_tasks(std::shared_ptr<warrior> w);
};
