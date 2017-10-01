/*
 * Timer.hpp
 * date:             02/15/2017
 * author:           Douglas Oliveira
 * last update:      10/01/2017
 * language version: C++11
 *
 * This header-only library contains some types of timers and alarms. It is very simple to use and it is useful
 * for performance evaluation and repeat tasks. The use, distribution and modification of this software is completly
 * free for academic, commercial and personal purposes.
 */

#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <functional>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

/* -------------------------------------------------------------------------------------- */
/* ------------------------------------- Classes List ----------------------------------- */
/* -------------------------------------------------------------------------------------- */

//! Defines the period used to measures the time (some popular specifications are made)
template <intmax_t _N, intmax_t _D, char ..._L> class Period;

//! Measures the elapsed time between two calls
template <class _Period> class Timer;
//! Measures the elapsed time in a block
template <class _Period> class BlockTimer;
//! Measures the elapsed time among many calls and computes statistical information
template <class _Period> class StatisticalTimer;

//! Gets the current time
class Clock;
//! Programs a function call to sometime in future
class Alarm;
//! Programs a function call that repeats every time interval
class PeriodicAlarm;


/////////////////////////////// internal use ///////////////////////////////
namespace internal {
namespace detail {
    template <class _Derived>
    struct is_time_period_impl {
        template <intmax_t a, intmax_t b, char ...c>
        static std::true_type test(const Period<a,b,c...>*);

        static std::false_type test(void*);

        using type = decltype(test(std::declval<_Derived*>()));
    };

    // create a string from a sequence of chars passed by variadic template
    template <char ..._String> std::string join_chars() {
        char s[sizeof...(_String)] { std::forward<char>(_String)... };
        s[sizeof...(_String)] = '\0';
        return std::string(s);
    }
} // detail

template <class _Derived>
using is_time_period = typename detail::is_time_period_impl<_Derived>::type;
} // internal
///////////////////////////////////////////////////////////////////////////

/* -------------------------------------------------------------------------------------- */
/* -------------------------------------- Definition ------------------------------------ */
/* -------------------------------------------------------------------------------------- */

/**
 *  @struct Period
 *  @brief Defines the period used to measures the time
 */
template <intmax_t _RatioNum, intmax_t _RatioDen, char ..._Label>
class Period {
public:
    typedef std::ratio<_RatioNum, _RatioDen> ratio;
    static const std::string& label() { return _label; }
private:
    static std::string _label;
};

// most popular specifications of Period
typedef Period <1,      1,          's'>           sec;
typedef Period <3600,   1,          'h'>           hour;
typedef Period <1,      1000000000, 'n','s'>       nanosec;
typedef Period <1,      1000000,    'u','s'>       microsec;
typedef Period <1,      1000,       'm','s'>       millisec;
typedef Period <60,     1,          'm','i','n'>   minute;


/**
 * @class Timer
 * @brief Measures the elapsed time between calls to start and stop methods
 * Use @code ostream_object << timer_object; @endcode to show the elapsed time
 */
template <class _Period>
class Timer {
public:
   void start();
   void stop();
   //! @return elapsed time
   double elapsed() const;

   static_assert(internal::is_time_period<_Period>::value, "Timer: invalid period");

private:
   std::chrono::time_point<std::chrono::high_resolution_clock> _start, _end;
};

/**
 * @class Block Timer
 * @brief Measures the elapsed time to the end of the block where the object was declared
 * Automatically shows the result in stream defined in constructor
 */
template <class _Period>
class BlockTimer {
public:
   BlockTimer(std::ostream& stream = std::cout);
   ~BlockTimer();

   static_assert(internal::is_time_period<_Period>::value, "BlockTimer: invalid period");

private:
   Timer<_Period> timer;
   std::ostream& out;
};

/**
 * @class Statistical Timer
 * @brief Measures and save the elapsed time between calls to start and save methods
 * Some statistical functions may be computed from the memorized times
 * Use @code ostream_object << timer_object; @endcode to show the results table (latex format)
 */
template <class _Period>
class StatisticalTimer {
public:
   void start();
   void stop();
   void save();
   void reset();

   double sum() const;
   double mean() const;
   double stdev() const;

   //! writes the data in latex table format string
   operator std::string() const;

   static_assert(internal::is_time_period<_Period>::value, "StatisticalTimer: invalid period");

private:
   Timer<_Period> timer;
   std::vector<double> memory;
};

/**
 * @class Clock
 * Use @code ostream_object << clock_object; @endcode to show the currently complete date/time
 * or @code string s(clock_object); @endcode to writes the date in a string
 */
struct Clock {
   //! return the amount of time since epoch
   template <class _Period = sec> static double now();
   //! return the number of ticks since epoch
   template <class _Period = nanosec> static time_t count();
   //! writes current local date/time in a string
   operator std::string() const;
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
    * @param wait   time to wait in milliseconds
    * @param rotine callable object that returns \b void (procedure, functor, lambda expression, bind expression etc)
    * @param args   arguments of the rotine
    * Ex: Lambda-expression: @code Alarm().program(2000, [] (int a, int b) { print(a+b); }, 4, 7); @endcode
    */
   template <class _Callable, class... _Args>
   void program(time_t wait, const _Callable& rotine, _Args... args);

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
   template<class _Callable, class... _Args>
   void program(time_t interval, const _Callable& rotine, _Args... args);

   //! cancel the alarm
   void cancel();

private:
   Alarm alarm;
};

/* -------------------------------------------------------------------------------------- */
/* ------------------------------------ Implementation ---------------------------------- */
/* -------------------------------------------------------------------------------------- */

//////////////////////////////////////////// TIMER /////////////////////////////////////////
template<class P> inline void Timer<P>::start() {
    _start = std::chrono::high_resolution_clock::now();
}
template<class P> inline void Timer<P>::stop() {
    _end = std::chrono::high_resolution_clock::now();
}
template<class P> inline double Timer<P>::elapsed() const {
    return std::chrono::duration_cast<std::chrono::duration<double, typename P::ratio>>(_end-_start).count();
}

template<class P> inline std::ostream& operator << (std::ostream& out, const Timer<P>& tm) {
   return (out << tm.elapsed() << P::label());
}

///////////////////////////////////////// BLOCKTIMER ////////////////////////////////////////
template<class P> inline BlockTimer<P>::BlockTimer(std::ostream& stream) : out(stream) { timer.start(); }
template<class P> inline BlockTimer<P>::~BlockTimer() { timer.stop(); out << "BlockTimer::elapsed: " << timer << "\n"; }

////////////////////////////////////// STATISTICALTIMER /////////////////////////////////////
template<class P> inline void StatisticalTimer<P>::start()   { timer.start(); }
template<class P> inline void StatisticalTimer<P>::stop()    { timer.stop(); }
template<class P> inline void StatisticalTimer<P>::save()    { timer.stop(); memory.push_back(timer.elapsed()); timer.start(); }
template<class P> inline void StatisticalTimer<P>::reset()   { timer = Timer<P>(); memory.clear(); }

template<class P> inline double StatisticalTimer<P>::sum() const {
   double _sum = 0.0;
   for(const double& xi : memory) _sum += xi;
   return _sum;
}

template<class P> inline double StatisticalTimer<P>::mean() const {
   return (!memory.empty()) ? sum()/memory.size() : 0.0;
}

template<class P> inline double StatisticalTimer<P>::stdev() const {
   double Ex  = mean();
   double Ex2 = 0.0;

   size_t N = memory.size();
   for(const double& xi : memory)
      Ex2 += xi * xi;
   if(!memory.empty()) Ex2/=N;

   return std::sqrt(Ex2 - (Ex * Ex));
}

template<class P> inline StatisticalTimer<P>::operator std::string() const {
    size_t k = 0;
    std::stringstream stream;
    for(const double& t : memory) {
        stream << "T" << ++k << " & " << t << P::label() << " \\\\ \n";
    }
    stream << "\\hline\n";
    stream << "Mean & " << mean() << P::label() << " \\\\ \n";
    stream << "Stdev & " << stdev() << P::label() << " \n";
    return stream.str();
}

template<class P> inline std::ostream& operator << (std::ostream& out, const StatisticalTimer<P>& timer) {
   return (out << std::string(timer));
}

//////////////////////////////////////////// CLOCK //////////////////////////////////////////
template<class P> inline double Clock::now() {
    static_assert(internal::is_time_period<P>::value, "Clock::now: invalid period");
    return std::chrono::duration_cast<std::chrono::duration<double, typename P::ratio>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}
template<class P> inline time_t Clock::count() {
    static_assert(internal::is_time_period<P>::value, "Clock::count: invalid period");
    return std::chrono::duration_cast<std::chrono::duration<time_t, typename P::ratio>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

inline Clock::operator std::string() const {
    time_t now = Clock::count<sec>();
    return std::string(std::asctime(std::localtime(&now)));
}
inline std::ostream& operator << (std::ostream& out, const Clock& clock) {
    return (out << std::string(clock));
}


//////////////////////////////////////////// ALARM //////////////////////////////////////////
inline Alarm::~Alarm() { cancel(); }

template <class _Callable, class... _Args> inline void Alarm::program(time_t wait, const _Callable& func, _Args... args) {
   if(!busy()) {
      std::function<void(_Args...)> event = func;
      background = std::thread([=] () {
         std::this_thread::sleep_for(std::chrono::milliseconds(wait));
         event(args...);
      });
   }
}
inline void Alarm::cancel()      { if(busy()) background.detach(); }
inline bool Alarm::busy() const  { return background.joinable(); }

/////////////////////////////////////// PERIODIC ALARM /////////////////////////////////////
template<class _Callable, class... _Args> inline void PeriodicAlarm::program(time_t interval, const _Callable& func, _Args... args) {
    std::function<void(_Args...)> event = func;
    alarm.program(interval, [=] () {
        event(args...);
        this->cancel();
        this->program(interval, event, args...);
    });
}
inline void PeriodicAlarm::cancel() { alarm.cancel(); }


/* -------------------------------------------------------------------------------------- */
/* --------------------------------------- Statics -------------------------------------- */
/* -------------------------------------------------------------------------------------- */

template <intmax_t _N, intmax_t _R, char ..._L>
std::string Period<_N, _R, _L...>::_label = internal::detail::join_chars<_L...>();

#endif // __TIMER_HPP__
