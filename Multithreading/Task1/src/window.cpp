#include "../include/window.hpp"

Window::Window() : shutdown_flag_ {false}
{
    initscr();
    curs_set(0);
    int term_width  = getmaxx(stdscr);
    int term_height = getmaxy(stdscr);
    window_         = newwin(term_height, term_width, 0, 0);
    key_watcher_ = std::thread([&](){
      pressed_exit();
    });
    wrefresh(window_);

}

Window::Window(int h_lines, int v_lines, int x_start, int y_start)
    : shutdown_flag_ {false}
{
    initscr();
    curs_set(0);
    window_ = newwin(v_lines, h_lines, x_start, y_start);
    wrefresh(window_);
}

Window& Window::operator=(Window&& other)
{
    if (window_ != other.window_)
    {
        delwin(window_);
        window_       = other.window_;
        other.window_ = nullptr;
    }
    return *this;
}

Window::Window(Window&& other)
{
    window_       = nullptr;
    window_       = other.window_;
    other.window_ = nullptr;
}

void Window::release_the_hounds()
{
    while (!shutdown_flag_.load())
    {
        balls.push_back(std::unique_ptr<Ball>(new Ball(
            std::chrono::milliseconds(40), window_)));
        wait_n_check_shutdwn(std::chrono::milliseconds(5000));
    }
}

void Window::wait_n_check_shutdwn(std::chrono::milliseconds wait_time)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start;
    auto duration = std::chrono::duration<double>(end - start);
    while(!shutdown_flag_.load() && duration < wait_time)
    {
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration<double>(end - start);
    }
}

void Window::pressed_exit()
{
    int ch;
    while(true)
    {
        ch = getch();
        if (ch == 27) // 27 - code for esc key
        {
            shutdown_flag_.store(true);
            break;
        }
    }
}

void Window::stop_all()
{
    key_watcher_.join();
    for (auto& ball : balls)
        ball->request_stop();
}

Window::~Window()
{
    stop_all();
    endwin();
}