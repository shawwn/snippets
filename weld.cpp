
#include "weld.h"

#include <algorithm>
#include <iostream>
#include <functional>
#include <vector>

struct Vec3 {
	Vec3() {}
	Vec3(const Vec3 & v) : x(v.x), y(v.y), z(v.z) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

	const Vec3 & operator =(const Vec3 & v) { x = v.x; y = v.y; z = v.z; return *this; }
	bool operator ==(const Vec3 & v) const { return x == v.x && y == v.y && z == v.z; }
	bool operator !=(const Vec3 & v) const { return x != v.x || y != v.y || z != v.z; }

	float x, y, z;
};

template <typename T>
struct print_function : public std::unary_function<T, void>
{
	print_function(std::ostream & out) : os(out) {}
	void operator() (T x) { os << x << ' '; }
	std::ostream & os;
};

template <>
struct print_function<Vec3> : public std::unary_function<Vec3, void>
{
	print_function(std::ostream & out) : os(out) {}
	void operator() (Vec3 v) { os << '(' << v.x << ' ' << v.y << ' ' << v.z << ") "; }
	std::ostream & os;
};

namespace std {
	template <> struct hash<Vec3> {
		size_t operator()(Vec3 v) { 
			const unsigned int * h = (const unsigned int *)(&v);
			unsigned int f = (h[0]+h[1]*11-(h[2]*17))&0x7fffffff;     // avoid problems with +-0
			return (f>>22)^(f>>12)^(f);
		}
	};
}


#if WELD_MAIN

int main()
{
	std::vector<int> int_vector;
	int_vector.push_back(1);
	int_vector.push_back(2);
	int_vector.push_back(3);
	int_vector.push_back(4);
	int_vector.push_back(5);
	int_vector.push_back(2);
	int_vector.push_back(4);
	int_vector.push_back(8);
	int_vector.push_back(10);
	int_vector.push_back(12);
	int_vector.push_back(1);

	for_each(int_vector.begin(), int_vector.end(), print_function<int>(std::cout));
	std::cout << std::endl;

	std::vector<size_t> xrefs;
	size_t num = weld(int_vector, xrefs, std::hash<int>(), std::equal_to<int>());

	for_each(int_vector.begin(), int_vector.end(), print_function<int>(std::cout));
	std::cout << std::endl;

	std::vector<const char *> str_vector;
	str_vector.push_back("monday");
	str_vector.push_back("tuesday");
	str_vector.push_back("thursday");
	str_vector.push_back("friday");
	str_vector.push_back("monday");
	str_vector.push_back("thursday");
	str_vector.push_back("tuesday");
	str_vector.push_back("wednesday");
	str_vector.push_back("wednesday");

	for_each(str_vector.begin(), str_vector.end(), print_function<const char *>(std::cout));
	std::cout << std::endl;

	num = weld(str_vector, xrefs, std::hash<const char *>(), std::equal_to<const char *>());

	for_each(str_vector.begin(), str_vector.end(), print_function<const char *>(std::cout));
	std::cout << std::endl;

	std::vector<Vec3> vec_vector;
	vec_vector.push_back( Vec3(0.0f, 1.0f, 0.0f) );
	vec_vector.push_back( Vec3(1.0f, 1.0f, 0.0f) );
	vec_vector.push_back( Vec3(0.0f, 1.0f, 1.0f) );
	vec_vector.push_back( Vec3(0.0f, 1.0f, 0.0f) );
	vec_vector.push_back( Vec3(0.0f, 1.0f, 1.0f) );
	vec_vector.push_back( Vec3(1.0f, 1.0f, 1.0f) );
	vec_vector.push_back( Vec3(1.0f, 1.0f, 0.0f) );

	for_each(vec_vector.begin(), vec_vector.end(), print_function<Vec3>(std::cout));
	std::cout << std::endl;

	num = weld(vec_vector, xrefs, std::hash<Vec3>(), std::equal_to<Vec3>());

	for_each(vec_vector.begin(), vec_vector.end(), print_function<Vec3>(std::cout));
	std::cout << std::endl;

	return 0;
}

#endif

