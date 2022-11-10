#include "pch_msc.hpp"

#include "io.h"
#include <limits>

#include <chrono>
#include <ctime>

#include "utils.h"
#include "libaquery.h"
#include <random>

char* gbuf = nullptr;

void setgbuf(char* buf) {
	static char* b = nullptr;
	if (buf == nullptr)
		gbuf = b;
	else {
		gbuf = buf;
		b = buf;
	}
}

#ifdef __AQ__HAS__INT128__

template <>
void print<__int128_t>(const __int128_t& v, const char* delimiter){
	char s[41];
	s[40] = 0;
	std::cout<< get_int128str(v, s+40)<< delimiter;
}
template <>
void print<__uint128_t>(const __uint128_t&v, const char* delimiter){
	char s[41];
	s[40] = 0;
    std::cout<< get_uint128str(v, s+40) << delimiter;
}
std::ostream& operator<<(std::ostream& os, __int128 & v)
{
	print(v);
	return os;
}
std::ostream& operator<<(std::ostream& os, __uint128_t & v)
{
	print(v);
	return os;
}

#endif

template <>
void print<bool>(const bool&v, const char* delimiter){
	std::cout<< (v?"true":"false") << delimiter;
}

template<class T> 
T getInt(const char*& buf){
	T ret = 0;
	while(*buf >= '0' and *buf <= '9'){
		ret = ret*10 + *buf - '0';
		buf++;
	}
	return ret;
}

template<class T> 
char* intToString(T val, char* buf){

	while (val > 0){
		*--buf = val%10 + '0';
		val /= 10;
	}
	
	return buf;
}
void skip(const char*& buf){ 
	while(*buf && (*buf >'9' || *buf < '0')) buf++; 
}

namespace types {

	date_t::date_t(const char* str) { fromString(str); }
	date_t& date_t::fromString(const char* str)  {
		if(str) {
			skip(str);
			year = getInt<short>(str);
			skip(str);
			month = getInt<unsigned char>(str);
			skip(str);
			day = getInt<unsigned char>(str);
		}
		else{
			day = month = year = 0;
		}
		return *this;
	}
	bool date_t::validate() const{
		return year <= 9999 && month <= 12 && day <= 31;
	}
	
	char* date_t::toString(char* buf) const {
		// if (!validate()) return "(invalid date)";
		*--buf = 0;
		buf = intToString(day, buf);
		*--buf = '-';
		buf = intToString(month, buf);
		*--buf = '-';
		buf = intToString(year, buf);
		return buf;
	}
	bool date_t::operator > (const date_t& other) const {
		return year > other.year || (year == other.year && (month > other.month || (month == other.month && day > other.day)));
	}

	bool date_t::operator < (const date_t& other) const {
		return year < other.year || (year == other.year && (month < other.month || (month == other.month && day < other.day)));
	}

	bool date_t::operator >= (const date_t& other) const {
		return year >= other.year || (year == other.year && (month >= other.month || (month == other.month && day >= other.day)));
	}

	bool date_t::operator <= (const date_t& other) const {
		return year <= other.year || (year == other.year && (month <= other.month || (month == other.month && day <= other.day)));
	}

	bool date_t::operator == (const date_t& other) const {
		return year == other.year && month == other.month && day == other.day;
	}

	bool date_t::operator != (const date_t& other) const {
		return !operator==(other);
	}

	time_t::time_t(const char* str) { fromString(str); }
	time_t& time_t::fromString(const char* str)  {
		if(str) {
			skip(str);
			hours = getInt<unsigned char>(str);
			skip(str);
			minutes = getInt<unsigned char>(str);
			skip(str);
			seconds = getInt<unsigned char>(str);
			skip(str);
			ms = getInt<unsigned int> (str);
		}
		else {
			hours = minutes = seconds = ms = 0;
		}
		return *this;
	}
	
	char* time_t::toString(char* buf) const {
		// if (!validate()) return "(invalid date)";
		*--buf = 0;
		buf = intToString(ms, buf);
		*--buf = ':';
		buf = intToString(seconds, buf);
		*--buf = ':';
		buf = intToString(minutes, buf);
		*--buf = ':';
		buf = intToString(hours, buf);
		return buf;
	}
	bool time_t::operator > (const time_t& other) const {
		return hours > other.hours || (hours == other.hours && (minutes > other.minutes || (minutes == other.minutes && (seconds > other.seconds || (seconds == other.seconds && ms > other.ms)))));
	}
	bool time_t::operator< (const time_t& other) const {
		return hours < other.hours || (hours == other.hours && (minutes < other.minutes || (minutes == other.minutes && (seconds < other.seconds || (seconds == other.seconds && ms < other.ms)))));
	} 
	bool time_t::operator>= (const time_t& other) const {
		return hours >= other.hours || (hours == other.hours && (minutes >= other.minutes || (minutes == other.minutes && (seconds >= other.seconds || (seconds == other.seconds && ms >= other.ms)))));
	}
	bool time_t::operator<= (const time_t& other) const{
		return hours <= other.hours || (hours == other.hours && (minutes <= other.minutes || (minutes == other.minutes && (seconds <= other.seconds || (seconds == other.seconds && ms <= other.ms)))));
	}
	bool time_t::operator==(const time_t& other) const {
		return hours == other.hours && minutes == other.minutes && seconds == other.seconds && ms == other.ms;
	}
	bool time_t::operator!= (const time_t& other) const {
		return !operator==(other);
	}
	bool time_t::validate() const{
		return hours < 24 && minutes < 60 && seconds < 60 && ms < 1000000;
	}

	timestamp_t::timestamp_t(const char* str) { fromString(str); }
	timestamp_t& timestamp_t::fromString(const char* str) {
		date.fromString(str);
		time.fromString(str);

		return *this;
	}
	bool timestamp_t::validate() const {
		return date.validate() && time.validate();
	}
	
	char* timestamp_t::toString(char* buf) const {
		buf = time.toString(buf);
		auto ret = date.toString(buf);
		*(buf-1) = ' ';
		return ret;
	}
	bool timestamp_t::operator > (const timestamp_t& other) const {
		return date > other.date || (date == other.date && time > other.time);
	}
	bool timestamp_t::operator < (const timestamp_t& other) const {
		return date < other.date || (date == other.date && time < other.time);
	}
	bool timestamp_t::operator >= (const timestamp_t& other) const {
		return date >= other.date || (date == other.date && time >= other.time);
	}
	bool timestamp_t::operator <= (const timestamp_t& other) const {
		return date <= other.date || (date == other.date && time <= other.time);
	}
	bool timestamp_t::operator == (const timestamp_t& other) const {
		return date == other.date && time == other.time;
	}
	bool timestamp_t::operator!= (const timestamp_t & other) const {
		return !operator==(other);
	}
}

template<class T>
void print_datetime(const T&v){
	char buf[T::string_length()];
	std::cout<<v.toString(buf + T::string_length());
}
std::ostream& operator<<(std::ostream& os, types::date_t & v)
{
	print_datetime(v);
	return os;
}
std::ostream& operator<<(std::ostream& os, types::time_t & v)
{
	print_datetime(v);
	return os;
}
std::ostream& operator<<(std::ostream& os, types::timestamp_t & v)
{
	print_datetime(v);
	return os;
}
std::ostream& operator<<(std::ostream& os, int8_t & v)
{
	os<<static_cast<int>(v);
	return os;
}
std::ostream& operator<<(std::ostream& os, uint8_t & v)
{
	os<<static_cast<unsigned>(v);
	return os;
}

std::string base62uuid(int l) {
    using namespace std;
    constexpr static const char* base62alp = 
		"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static mt19937_64 engine(
		std::chrono::system_clock::now().time_since_epoch().count());
    static uniform_int_distribution<uint64_t> u(0x10000, 0xfffff);
    uint64_t uuid = (u(engine) << 32ull) + 
		(std::chrono::system_clock::now().time_since_epoch().count() & 0xffffffff);
    //printf("%llu\n", uuid);
    string ret;
    while (uuid && l-- >= 0) {
        ret = string("") + base62alp[uuid % 62] + ret;
        uuid /= 62;
    }
    return ret;
}

template <>
inline const char* str(const bool& v) {
	return v ? "true" : "false";
}


Context::Context() {
    current.memory_map = new std::unordered_map<void*, deallocator_t>;
    init_session();
}

Context::~Context() {
    auto memmap = (std::unordered_map<void*, deallocator_t>*) this->current.memory_map;
    delete memmap;
}

void Context::init_session(){
    if (log_level == LOG_INFO){
        memset(&(this->current.stats), 0, sizeof(Session::Statistic));
    }
    auto memmap = (std::unordered_map<void*, deallocator_t>*) this->current.memory_map;
    memmap->clear();
}

void Context::end_session(){
    auto memmap = (std::unordered_map<void*, deallocator_t>*) this->current.memory_map;
    for (auto& mem : *memmap) {
        mem.second(mem.first);
    }
    memmap->clear();
}

void* Context::get_module_function(const char* fname){
    auto fmap = static_cast<std::unordered_map<std::string, void*>*>
        (this->module_function_maps);
    // printf("%p\n", fmap->find("mydiv")->second);
    //  for (const auto& [key, value] : *fmap){
    //      printf("%s %p\n", key.c_str(), value);
    //  }
    auto ret = fmap->find(fname);
    return ret == fmap->end() ? nullptr : ret->second;
}
// template<typename _Ty>
// inline void vector_type<_Ty>::out(uint32_t n, const char* sep) const
// {
// 	n = n > size ? size : n;
// 	std::cout << '(';
// 	{	
// 		uint32_t i = 0;
// 		for (; i < n - 1; ++i)
// 			std::cout << this->operator[](i) << sep;
// 		std::cout << this->operator[](i);
// 	}
// 	std::cout << ')';
// }

#include "gc.h"
#include <vector_type>
#include <utility>
#include <thread>
#include <chrono>
#ifndef __AQ_USE_THREADEDGC__

struct gcmemory_t{
	void* memory;
	void (*deallocator)(void*);
};

using memoryqueue_t = gcmemory_t*;
void GC::acquire_lock() {
	auto this_tid = std::this_thread::get_id();
	while(lock != this_tid)
	{
		while(lock != this_tid && lock != std::thread::id()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(0));
		}
		lock = this_tid;
	}
}

void GC::release_lock(){
	lock = std::thread::id();
}

void GC::gc()
{
	auto& _q = static_cast<memoryqueue_t*>(q);
	auto& _q_back = static_cast<memoryqueue_t*>(q_back);
	if (_q->size == 0)
		return;
	auto t = _q;
	lock = true;
	while(alive_cnt > 0);
	_q = q_back;
	uint32_t _slot = slot_pos;
	slot_pos = 0;
	current_size = 0;
	lock = false;
	_q_back = t;

	for(uint32_t i = 0; i < _slot; ++i){
		if (_q_back[i]->memory != nullptr && _q_back[i]->deallocator != nullptr)
			_q_back[i]->deallocator(_q_back[i]->memory);
	}
	memset(_q_back, 0, sizeof(gcmemory_t) * _slot);
	running = false;
}

void GC::daemon() {
	using namespace std::chrono;

	while (alive) {
		if (running) {
			if (current_size > max_size || 
				forceclean_timer > forced_clean) 
			{
				gc();
				forceclean_timer = 0;
			}
			std::this_thread::sleep_for(microseconds(interval));
			forceclean_timer += interval;
		}
		else {
			std::this_thread::sleep_for(10ms);
			forceclean_timer += 10000;
		}
	}
}

void GC::start_deamon() {
	q = new gcmemory_t[max_slots << 1];
	q_back = new memoryqueue_t[max_slots << 1];
	lock = false;
	slot_pos = 0;
	current_size = 0;
	alive_cnt = 0;
	alive = true;
	handle = new std::thread(&GC::daemon, this);
}

void GC::terminate_daemon() {
	running = false;
	alive = false;
	decltype(auto) _handle = static_cast<std::thread*>(handle);
	delete[] static_cast<memoryqueue_t>(q);
	delete[] static_cast<memoryqueue_t>(q_back);
	using namespace std::chrono;
	std::this_thread::sleep_for(microseconds(1000 + std::max(static_cast<size_t>(10000), interval)));

	if (_handle->joinable()) {
		_handle->join();
	}
	delete _handle;
}

void GC::reg(void* v, uint32_t sz, void(*f)(void*)) { //~ 40ns expected v. free ~ 75ns
	if (v == nullptr || f == nullptr)
		return;
	if (sz < threshould){
		f(v);
		return;
	}
	auto _q = static_cast<memoryqueue_t>q;
	while(lock);
	++alive_cnt;
	current_size += sz;
	auto _slot = (slot_pos += 1);
	q[_slot] = {v, f};
	--alive_cnt;
	running = true;
}

#endif

static GC* GC::gc = nullptr;