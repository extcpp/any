#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_any
#include <boost/test/unit_test.hpp>
#include "any/any.hpp"

BOOST_AUTO_TEST_CASE(test_any_int)
{
	any::any<16, 8> a = 42;
	BOOST_TEST(any::valid_cast<int>(a));
	auto result = any::any_cast<int>(a);
	BOOST_TEST(result == 42);
}

struct dummy
{
	static bool copy;
	static bool copy_assign;
	static bool move;
	static bool move_assign;
	static bool dtor;

	dummy(){ }
	dummy(dummy const&){ copy = true; }
	dummy& operator= (dummy const&){ copy_assign = true; return *this;}
	dummy(dummy&&){ move = true; }
	dummy& operator= (dummy&&){ move_assign = true; return *this;}
	~dummy(){ dtor = true; }
};
bool dummy::copy = false;
bool dummy::copy_assign = false;
bool dummy::move = false;
bool dummy::move_assign = false;
bool dummy::dtor = false;


BOOST_AUTO_TEST_CASE(test_any_dummy_class)
{
	{
		dummy d{};
		any::any<16, 8> a1 = d;
		dummy::copy        = false;
		dummy::copy_assign = false;
		dummy::move        = false;
		dummy::move_assign = false;
		dummy::dtor        = false;
		any::any<16,8> a2 = a1;
		BOOST_TEST(dummy::copy        == true);
		BOOST_TEST(dummy::copy_assign == false);
		BOOST_TEST(dummy::move        == false);
		BOOST_TEST(dummy::move_assign == false);
		BOOST_TEST(dummy::dtor        == false);
	}
	BOOST_TEST(dummy::dtor == true);

	{
		dummy d{};
		any::any<16, 8> a1;
		any::any<16, 8> a2 = d;
		dummy::copy        = false;
		dummy::copy_assign = false;
		dummy::move        = false;
		dummy::move_assign = false;
		dummy::dtor        = false;
		a1 = a2;
		BOOST_TEST(dummy::copy        == true);
		BOOST_TEST(dummy::copy_assign == false);
		BOOST_TEST(dummy::move        == false);
		BOOST_TEST(dummy::move_assign == false);
		BOOST_TEST(dummy::dtor        == false);
	}
	BOOST_TEST(dummy::dtor == true);

	{
		any::any<16, 8> a1 = dummy{};
		dummy::copy        = false;
		dummy::copy_assign = false;
		dummy::move        = false;
		dummy::move_assign = false;
		dummy::dtor        = false;
		any::any<16, 8> a2 = std::move(a1);
		BOOST_TEST(dummy::copy        == false);
		BOOST_TEST(dummy::copy_assign == false);
		BOOST_TEST(dummy::move        == true);
		BOOST_TEST(dummy::move_assign == false);
		BOOST_TEST(dummy::dtor        == false);
	}
	BOOST_TEST(dummy::dtor == true);

	{
		dummy d{};
		any::any<16, 8> a1;
		any::any<16, 8> a2 = d;
		dummy::copy        = false;
		dummy::copy_assign = false;
		dummy::move        = false;
		dummy::move_assign = false;
		dummy::dtor        = false;
		a1 = std::move(a2);
		BOOST_TEST(dummy::copy        == false);
		BOOST_TEST(dummy::copy_assign == false);
		BOOST_TEST(dummy::move        == true);
		BOOST_TEST(dummy::move_assign == false);
		BOOST_TEST(dummy::dtor        == false);
	}
	BOOST_TEST(dummy::dtor == true);

}
