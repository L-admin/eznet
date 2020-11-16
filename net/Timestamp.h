#pragma once

#include <stdint.h>
#include <algorithm>
#include <string>

using namespace std;

namespace net
{
	class Timestamp
	{
	public:
		Timestamp() : microSecondsSinceEpoch_(0)
		{
		}

		explicit Timestamp(int64_t microSecondsSinceEpoch);

		// default copy/assignment/dtor are Okay

		Timestamp &operator+=(Timestamp lhs)
		{
			this->microSecondsSinceEpoch_ += lhs.microSecondsSinceEpoch_;
			return *this;
		}

		Timestamp &operator+=(int64_t lhs)
		{
			this->microSecondsSinceEpoch_ += lhs;
			return *this;
		}

		Timestamp &operator-=(Timestamp lhs)
		{
			this->microSecondsSinceEpoch_ -= lhs.microSecondsSinceEpoch_;
			return *this;
		}

		Timestamp &operator-=(int64_t lhs)
		{
			this->microSecondsSinceEpoch_ -= lhs;
			return *this;
		}

		void swap(Timestamp &that)
		{
			std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
		}

		string toString() const;

		string toFormattedString(bool showMicroseconds = true) const;

		bool valid() const { return microSecondsSinceEpoch_ > 0; }

		// for internal usage.
		int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

		time_t secondsSinceEpoch() const
		{
			return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
		}

		static Timestamp now();

		static const int kMicroSecondsPerSecond = 1000 * 1000;

	private:
		int64_t microSecondsSinceEpoch_;
	};

	inline bool operator<(Timestamp lhs, Timestamp rhs)
	{
		return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
	}

	inline bool operator>(Timestamp lhs, Timestamp rhs)
	{
		return rhs < lhs;
	}

	inline bool operator<=(Timestamp lhs, Timestamp rhs)
	{
		return !(lhs > rhs);
	}

	inline bool operator>=(Timestamp lhs, Timestamp rhs)
	{
		return !(lhs < rhs);
	}

	inline bool operator==(Timestamp lhs, Timestamp rhs)
	{
		return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
	}

	inline bool operator!=(Timestamp lhs, Timestamp rhs)
	{
		return !(lhs == rhs);
	}

	inline double timeDifference(Timestamp high, Timestamp low)
	{
		int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
		return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
	}

	inline Timestamp addTime(Timestamp timestamp, int64_t microseconds)
	{
		return Timestamp(timestamp.microSecondsSinceEpoch() + microseconds);
	}
} // namespace net