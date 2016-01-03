#pragma once
#ifndef PARANOISE_INTRINSIC_M256I_H
#define PARANOISE_INTRINSIC_M256I_H

#include "base.h"

namespace paranoise {	namespace parallel {

	union m256f;
	union m256d;

	ALIGN(32) union m256i {
		__m256i val;

		int64 i64[4];
		int32 i32[8];
		int16 i16[16];
		int8  i8[32];

		m256i() = default;
		m256i(const int32& rhs)		{ val = _mm256_set1_epi32(rhs); }

		m256i(const __m256& rhs)	{ val = _mm256_cvtps_epi32(rhs); }
		m256i(const __m256i& rhs)	{ val = rhs; }
		m256i(const __m256d& rhs)	{ val = _mm256_castpd_si256(rhs); }

		m256i(const m256f&	rhs);
		m256i(const m256i&	rhs);
		m256i(const m256d&	rhs);
	};

	inline m256i operator +(const m256i& a, const m256i& b) { return _mm256_add_epi32	(a.val, b.val); }
	inline m256i operator -(const m256i& a, const m256i& b) { return _mm256_sub_epi32	(a.val, b.val); }
	inline m256i operator *(const m256i& a, const m256i& b) { return _mm256_mul_epi32	(a.val, b.val); }
	

	inline m256i operator >(const m256i& a, const m256i& b) { return _mm256_cmpgt_epi32	(a.val, b.val); }
	inline m256i operator <(const m256i& a, const m256i& b) { return _mm256_cmpgt_epi32	(b.val, a.val); }
	inline m256i operator==(const m256i& a, const m256i& b) { return _mm256_cmpeq_epi32 (a.val, b.val); }

	inline m256i operator |(const m256i& a, const m256i& b) { return _mm256_or_si256	(a.val, b.val); }
	inline m256i operator &(const m256i& a, const m256i& b) { return _mm256_and_si256	(a.val, b.val); }
	inline m256i operator ^(const m256i& a, const m256i& b) { return _mm256_xor_si256	(a.val, b.val); }
	inline m256i operator ~(const m256i& a)				    { return _mm256_andnot_si256(a.val, a.val); }

	
	inline m256i min(const m256i& a, const m256i& b)		{ return _mm256_min_epi32	(a.val, b.val); }
	inline m256i max(const m256i& a, const m256i& b)		{ return _mm256_max_epi32	(a.val, b.val); }	
}}

#endif