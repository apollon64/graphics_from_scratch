/**
  * C++ memory bandwidth test.
  * Compilation command: gcc mem.cpp -lstdc++ -ggdb -O3 -o prog
  * @author Torbjorn Haugen, 
  * based on a gist by Andrea Catania https://gist.github.com/AndreaCatania/31778caae7c3844fcacfbd969d3074ee
  */

#include "spinners.hpp" //https://github.com/jkuri/spinners
#include <immintrin.h>
#include <assert.h>
#include <float.h>
#include <chrono>
#include <algorithm>
#include <stdio.h>
#include <string>
#include <vector>

using U8 = unsigned char;
using U16 = unsigned short;
using U32 = unsigned int;
using U64 = unsigned long long;

// SSE types are intrisically arrays and need some work to behave as wide integers.
//typedef __m128i uint128_t;
//typedef __m256i uint256_t;

#define ArrayLen(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

// 4x4px blocks? 8x8px blocks palettized to 256?
using BlockType = U8;
static const int ARR_SIZE = 2*1024;
struct AssignableArray{ BlockType m[ARR_SIZE]; } ;

static const AssignableArray zeros = {0};
// 32kB
// 4bytes * 16
struct Block {
	AssignableArray m;
	Block() 
	{
		m = zeros;
		//for(int i=0; i<ArrayLen(m.m); i++) { m.m[i] = 0; }
	}
	Block(int x) 
	{
		for(int i=0; i<ArrayLen(m.m); i++) { m.m[i] = (BlockType)x; }
	}
	Block(AssignableArray x) : m(x)
	{
	}
	BlockType& operator[](int i)
	{
		return m.m[i];
	}
	BlockType operator[](int i) const
	{
		return m.m[i];
	}
	Block& operator=(int x)
	{
		for(int i=0; i<ArrayLen(m.m); i++) {
			m.m[i] = (BlockType) x;
		}
		return *this;
	}
	Block& operator=(const Block& other)
	{
		for(int i=0; i<ArrayLen(m.m); i++) {
			m.m[i] = other[i];
		}
		return *this;
	}
	Block& operator+=(const Block& other)
	{
		for(int i=0; i<ArrayLen(m.m); i++) {
			(*this)[i] += other[i];
		}
		return *this;
	}
	bool operator==(Block& other)
	{
		for(int i=0; i<ArrayLen(m.m); i++) {
			if ((*this)[i] != other[i] )  return false;
		}
		return true;
	}
};

Block operator+(const Block& a, const Block& b)
{
	AssignableArray sum;
	for(int i=0; i<ArrayLen(sum.m); i++) {
		sum.m[i] = a[i] + b[i];
	}
	return Block(sum);
}
Block operator*(const Block& a, const Block& b)
{
	AssignableArray sum;
	for(int i=0; i<ArrayLen(sum.m); i++) {
		sum.m[i] = a[i] * b[i];
	}
	return Block(sum);
}

int main() {
	const int NUM_KERNELS = 7;
	const char* kernel_names[] =
	 {"copy    :", 
	  "scale   :",
	  "add     :",
	  "mad     :",
	  "forward :",
	  "reverse :", 
	  "random  :" };
	int accesses[NUM_KERNELS];
	double time_spent[NUM_KERNELS];

	for(int i=0; i<NUM_KERNELS; i++) {
		accesses[i] = 0;
		time_spent[i] = DBL_MAX;
	}

	using STREAM_TYPE = Block;
	const unsigned elem_size = sizeof(STREAM_TYPE);
	const unsigned int arr_size = (800*600)/ARR_SIZE; //10000000; //1920 * 1080;
	const unsigned int iterations = 10;
	const unsigned bytes = elem_size * arr_size;
	printf("Size of element:%d bytes\n", elem_size );
	printf("Size of one array:%f MB\n", bytes / 1024.0 / 1024.0 );
	printf("Total data:%f MB\n", 3 * bytes / 1024.0 / 1024.0 );

	std::vector<STREAM_TYPE> array_a(arr_size);
	std::vector<STREAM_TYPE> array_b(arr_size);
	std::vector<STREAM_TYPE> array_c(arr_size);
	//STREAM_TYPE *array_a = new STREAM_TYPE[arr_size];
	//STREAM_TYPE *array_b = new STREAM_TYPE[arr_size];
	//STREAM_TYPE *array_c = new STREAM_TYPE[arr_size];

	jms::Spinner s("Benchmarking...", jms::bounce);
  	s.start();
	STREAM_TYPE sum_copy{0};
	STREAM_TYPE sum_forward{0};
	STREAM_TYPE sum_reverse{0};
	STREAM_TYPE sum_random{0};

	for (unsigned int i = 0; i < arr_size; ++i) {
	  array_a[i] = rand() % 127;
	  array_b[i] = rand() % 127;
	  array_c[i] = 0;
	}
	auto start = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> end_time = std::chrono::high_resolution_clock::now() - start;

	int bench = 0;
	{ // Copy
		bench = 0; accesses[bench] = 2;
		for(int x = 0; x < iterations; x++) {
			start = std::chrono::high_resolution_clock::now();
			for (U32 i = 0; i < arr_size; ++i) {
			  array_c[i] = array_a[i];
			}
			end_time = (std::chrono::high_resolution_clock::now() - start);
			time_spent[bench] = std::min<double>( time_spent[bench], end_time.count() );
		}
	}

	for (U32 i = 0; i < arr_size; ++i) { 
		assert(array_c[i] == array_a[i]);
	}

	{ // Scale
		bench = 1; accesses[bench] = 3;
		for(int x = 0; x < iterations; x++) {
			start = std::chrono::high_resolution_clock::now();
			for (unsigned int i = 0; i < arr_size; ++i) {
			  array_c[i] = array_a[i] * array_b[i];
			}
			end_time = (std::chrono::high_resolution_clock::now() - start);
			time_spent[bench] = std::min<double>( time_spent[bench], end_time.count() );
		}	
	}

	{ // Add
		bench = 2; accesses[bench] = 3;
		for(int x = 0; x < iterations; x++) {
			start = std::chrono::high_resolution_clock::now();
			for (unsigned int i = 0; i < arr_size; ++i) {
			  array_c[i] = array_a[i] + array_b[i];
			}
			end_time = (std::chrono::high_resolution_clock::now() - start);
			time_spent[bench] = std::min<double>( time_spent[bench], end_time.count() );
		}	
	}

	{ // MAD
		bench = 3; accesses[bench] = 3;
		for(int x = 0; x < iterations; x++) {
			start = std::chrono::high_resolution_clock::now();
			for (unsigned int i = 0; i < arr_size; ++i) {
			  array_c[i] = array_a[i] * array_a[i] * array_b[i];
			}
			end_time = (std::chrono::high_resolution_clock::now() - start);
			time_spent[bench] = std::min<double>( time_spent[bench], end_time.count() );
		}	
	}

	{ // Forward
		bench = 4; accesses[bench] = 2;
		for(int x = 0; x < iterations; x++) {
			start = std::chrono::high_resolution_clock::now();
			for (unsigned int i = 0; i < arr_size; ++i) {
			  sum_forward += array_a[i] + array_b[i];
			}
			end_time = (std::chrono::high_resolution_clock::now() - start);
			time_spent[bench] = std::min<double>( time_spent[bench], end_time.count() );
		}
	}

	{ // Reverse access
		bench = 5; accesses[bench] = 2;
		for(int x = 0; x < iterations; x++)
		{
			start = std::chrono::high_resolution_clock::now();
			for (int i = arr_size - 1; i >= 0; --i) {
				sum_reverse += array_a[i] + array_b[i];
			}
			end_time = (std::chrono::high_resolution_clock::now() - start);
			time_spent[bench] = std::min<double>( time_spent[bench], end_time.count() );
		}
	}

	{ // Random access
		bench = 6; accesses[bench] = 2;
		for(int x = 0; x < iterations; x++)
		{
			start = std::chrono::high_resolution_clock::now();
			for (unsigned int i = 0; i < arr_size; ++i) {
				const int p(rand() % arr_size);
				sum_random += array_a[p] + array_b[p];
			}
			end_time = (std::chrono::high_resolution_clock::now() - start);
			time_spent[bench] = std::min<double>( time_spent[bench], end_time.count() );
		}
	}
  	s.finish(jms::FinishedState::SUCCESS, "Benchmarks complete.");

	printf("Kernel:    Best time:   best MB/s: \n");
	for(int i=0; i<NUM_KERNELS; i++) {
		printf("%s%.6f sec   %.3f\n", kernel_names[i], time_spent[i], 1.0E-06 * accesses[i]*bytes / time_spent[i] );
	}
	assert(sum_forward == sum_reverse);
	for(int i=0; i<ArrayLen(sum_random.m.m) && i%2==0; i++) {
		printf("%d", sum_random[i]);
	}
	//printf("END PROGRAM sum:%d %d", sum_forward== sum_reverse, sum_random);

	return 0;
}