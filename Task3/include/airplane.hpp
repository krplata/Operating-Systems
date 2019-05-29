#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include "priority.hpp"
#include "route.hpp"
#include "utils.hpp"
#include "window.hpp"

class Airplane
{
   public:
    enum class Action
    {
        OUTGOING,
        INCOMING
    };

    Airplane() = delete;
    Airplane(std::chrono::milliseconds speed, Window& win, Priority p)
        : speed_ {speed}, win_ {win}, priority_ {p}
    {
        can_move_to_pa_.store(false);
        can_move_to_runway_.store(false);
    };

    virtual ~Airplane() = default;

    // Take off or Landing, depending on the base class implementation.
    virtual void start_action()            = 0;  // Better name ?
    virtual Action get_action_type() const = 0;

    bool operator>(const Airplane& b) const
    {
        return this->priority_.get_priority() > b.priority_.get_priority();
    };

    void allow_move_to_pa() { can_move_to_pa_.store(true); };
    void allow_move_to_runway() { can_move_to_runway_.store(true); };
    bool has_finished_action() { return finished_action_.load(); }
    void set_route(Route route)
    {
        route_    = route;
        position_ = route_.start_;
    }

   protected:
    std::chrono::milliseconds speed_;
    Window& win_;
    std::pair<size_t, size_t> position_;
    Priority priority_;
    Route route_;

    std::atomic_bool can_move_to_pa_;
    std::atomic_bool can_move_to_runway_;
    std::atomic_bool finished_action_;

    void move_horizontally(std::pair<size_t, size_t>& prev,
                           const std::pair<size_t, size_t> next)
    {
        int x_diff      = prev.first - next.first;
        auto func       = utils::get_operator_for_sign(x_diff);
        auto iterations = abs(x_diff);
        for (int moves = 1; moves <= iterations; ++moves)
        {
            win_.move_on_screen(prev, {func(prev.first), prev.second});
            prev.first = func(prev.first);
            std::this_thread::sleep_for(std::chrono::milliseconds(speed_));
        }
    }

    void move_vertically(std::pair<size_t, size_t>& prev,
                         const std::pair<size_t, size_t> next)
    {
        int y_diff      = prev.second - next.second;
        auto func       = utils::get_operator_for_sign(y_diff);
        auto iterations = abs(y_diff);
        for (int moves = 1; moves <= iterations; ++moves)
        {
            win_.move_on_screen(prev, {prev.first, func(prev.second)});
            prev.second = func(prev.second);
            std::this_thread::sleep_for(std::chrono::milliseconds(speed_));
        }
    }
};