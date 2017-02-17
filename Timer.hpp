/*
 * Timer.hpp
 * date:             02/15/2017
 * author:           Douglas Oliveira
 * last update:      02/17/2017
 * language version: C++14
 *
 * This header-only library contains some types of timers and alarms. It is very simple to use and it is useful
 * for performance evaluation and repeat tasks. The use, distribution and modification of this software is completly
 * free for academic, commercial and personal purposes. No metion nor citation is needed.
 */

#ifndef AVR_TIMER_HPP
#define AVR_TIMER_HPP

#include <functional>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <ctime>

namespace avr {
using namespace std::chrono;

/**
 * @class Timer
 * @brief Measures the elapsed time between calls to start and stop methods
 * Use @code cout << timer_object; @endcode to show the elapsed time in seconds
 */
class Timer {
public:
   void start();
   void stop();
   // elapsed seconds
   double elapsed() const;

private:
   time_point<high_resolution_clock> _start, _end;
};

/**
 * @class Block Timer
 * @brief Measures the elapsed time to the end of the block where the object was declared
 * Automatically shows the result in stream defined in constructor
 */
class BlockTimer {
public:
   BlockTimer(std::ostream& stream = std::cout);
   ~BlockTimer();

private:
   Timer timer;
   std::ostream& out;
};

/**
 * @class Statistical Timer
 * @brief Measures and save the elapsed time between calls to start and save methods
 * Some statistical functions may be computed from the memorized times
 * Use @code cout << timer_object; @endcode to show the results table (latex format)
 */
class StatisticalTimer {
public:
   void start();
   void stop();
   void save();
   void reset();

   double sum() const;
   double mean() const;
   double stdev() const;

   friend std::ostream& operator << (std::ostream& out, const StatisticalTimer& timer);

private:
   Timer timer;
   std::vector<double> memory;
};

/**
 * @class Clock
 * @brief Returns the count of ticks
 * Use @code cout << clock_object; @endcode to show the currently complete hour (HH:MM:SS)
 */
struct Clock {
   time_t operator() () const;
};

/**
 * @class Alarm
 * @brief Program a alarm passing the time to wait in milliseconds
 * After waiting a signal is dispached calling a configurable rotine
 * @note Only one alarm may be set at a time
 */
class Alarm {
public:
   ~Alarm();

   /** Program the alarm
    * @param wait   time to wait in ms
    * @param rotine pointer to a procedure or a functor or a lambda expression (both must return void)
    * @param args   arguments of the rotine
    * Ex: Lambda-expression: @code Alarm().program(2000, [] (int a, int b) { print(a+b); }, 4, 7); @endcode
    */
   template <class... Args>
   void program(time_t wait, const auto& rotine, Args... args);

   //! cancel the currently alarm
   void cancel();

   //! checks the alarm is busy
   bool busy() const;

private:
   std::thread background;
};

/**
 * @class Periodic Alarm
 * @brief Automatically reprogramming the alarm when it is finished
 */
class PeriodicAlarm {
public:

   //! program the alarm (see Alarm::program for details)
   template<class... Args>
   void program(time_t interval, const auto& rotine, Args... args);

   //! cancel the alarm
   void cancel();

private:
   Alarm alarm;
};

} // avr

//! Implementation !//

namespace avr {
//////////////////////////////////////////// TIMER ////////////////////////////////////////////
inline void Timer::start() { _start = high_resolution_clock::now(); }
inline void Timer::stop()  { _end = high_resolution_clock::now(); }
inline double Timer::elapsed() const { return duration<double>(_end-_start).count(); }

inline std::ostream& operator << (std::ostream& out, const Timer& tm) {
   return (out << tm.elapsed() << "s");
}

///////////////////////////////////////// BLOCKTIMER /////////////////////////////////////////
inline BlockTimer::BlockTimer(std::ostream& stream) : out(stream) { timer.start(); }
inline BlockTimer::~BlockTimer() { timer.stop(); out << "Elapsed: " << timer << "\n"; }

////////////////////////////////////// STATISTICALTIMER //////////////////////////////////////
inline void StatisticalTimer::start()   { timer.start(); }
inline void StatisticalTimer::stop()    { timer.stop(); }
inline void StatisticalTimer::save()    { stop(); memory.push_back(timer.elapsed()); start(); }
inline void StatisticalTimer::reset()   { timer = Timer(); memory.clear(); }

inline double avr::StatisticalTimer::sum() const {
   double _sum = 0.0;
   for(const double& xi : memory) _sum += xi;
   return _sum;
}

inline double avr::StatisticalTimer::mean() const {
   return (!memory.empty()) ? sum()/memory.size() : 0.0;
}

inline double avr::StatisticalTimer::stdev() const {
   double Ex  = mean();
   double Ex2 = 0.0;

   size_t N = memory.size();
   for(const double& xi : memory)
      Ex2 += xi * xi;
   if(!memory.empty()) Ex2/=N;

   return std::sqrt(Ex2 - (Ex * Ex));
}

inline std::ostream& operator << (std::ostream& out, const StatisticalTimer& timer) {
   size_t k = 0;
   for(const double& t : timer.memory) {
      out << "T" << ++k << " & " << t << "s \\\\ \n";
   }
   out << "\\hline\n";
   out << "Mean & " << timer.mean() << "s \\\\ \n";
   out << "Stdev & " << timer.stdev() << "s \n";
   return out;
}

//////////////////////////////////////////// CLOCK ////////////////////////////////////////////
inline time_t Clock::operator() () const {
   return system_clock::to_time_t(high_resolution_clock::now());
}

inline std::ostream& operator << (std::ostream& out, const Clock& clock) {
   time_t now = clock(); char buf[32];

   time(&now);
   strftime(buf, 32, "Now it's %T", localtime(&now));
   return (out << buf);
}

//////////////////////////////////////////// ALARM ////////////////////////////////////////////
inline Alarm::~Alarm() { cancel(); }

template <class... Args> inline void Alarm::program(time_t wait, const auto& func, Args... args) {
   if(!busy()) {
      std::function<void(Args...)> event = func;
      background = std::thread([=] () {
         std::this_thread::sleep_for(milliseconds(wait));
         event(args...);
      });
   }
}
inline void Alarm::cancel()      { if(busy()) background.detach(); }
inline bool Alarm::busy() const  { return background.joinable(); }


/////////////////////////////////////// PERIODIC ALARM ///////////////////////////////////////
template<class... Args> inline void PeriodicAlarm::program(time_t interval, const auto& func, Args... args) {
   std::function<void(Args...)> event = func;
   alarm.program(interval, [=] () { event(args...); this->cancel(); this->program(interval, event, args...); });
}
inline void PeriodicAlarm::cancel() { alarm.cancel(); }

}


#endif // AVR_TIMER_HPP
