#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_any
#include <boost/test/unit_test.hpp>
#include "any/any.hpp"

BOOST_AUTO_TEST_CASE(test_is_any)
{
	static_assert(!any::detail::is_any<int>::value);
	static_assert( any::detail::is_any<any::base_any<16, 8>>::value);
}

BOOST_AUTO_TEST_CASE(test_any_int)
{
	using any_t = any::any<16, 8>;

	any_t a = 42;
	BOOST_TEST(any::valid_cast<int>(a) == true);

	decltype(auto) result = any::any_cast<int>(a);
	static_assert(std::is_same<decltype(result), int&>::value);
	BOOST_TEST(result == 42);

	result = 170;
	BOOST_TEST(any::any_cast<int>(a) == 170);

	any_t const& b = a;
	BOOST_TEST(any::valid_cast<int>(b) == true);
	BOOST_TEST(any::any_cast<int>(b) == 170);

	decltype(auto) result2 = any::any_cast<int>(b);
	static_assert(std::is_same<decltype(result2), int const&>::value);
	BOOST_TEST(result2 == 170);

	result = 682;
	BOOST_TEST(result2 == 682);
}

struct dummy
{
	static unsigned copy;
	static unsigned copy_assign;
	static unsigned move;
	static unsigned move_assign;
	static unsigned dtor;

	dummy(){ }
	dummy(dummy const&){ ++copy; }
	dummy& operator= (dummy const&){ ++copy_assign; return *this;}
	dummy(dummy&&){ ++move; }
	dummy& operator= (dummy&&){ ++move_assign; return *this;}
	~dummy(){ ++dtor; }
};
unsigned dummy::copy        = 0;
unsigned dummy::copy_assign = 0;
unsigned dummy::move        = 0;
unsigned dummy::move_assign = 0;
unsigned dummy::dtor        = 0;


BOOST_AUTO_TEST_CASE(test_any_copy_move)
{
	using any_t = any::base_any<16, 8, any::iface::copy, any::iface::move>;
	{
		dummy d{};
		any_t a1 = d;
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		any_t a2 = a1;
		BOOST_TEST(dummy::copy        == 1);
		BOOST_TEST(dummy::copy_assign == 0);
		BOOST_TEST(dummy::move        == 0);
		BOOST_TEST(dummy::move_assign == 0);
		BOOST_TEST(dummy::dtor        == 0);
	}
	BOOST_TEST(dummy::dtor == 3);

	{
		dummy d{};
		any_t a1 = d;
		any_t a2 = d;
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		a1 = a2;
		BOOST_TEST(dummy::copy        == 1);
		BOOST_TEST(dummy::copy_assign == 0);
		BOOST_TEST(dummy::move        == 0);
		BOOST_TEST(dummy::move_assign == 0);
		BOOST_TEST(dummy::dtor        == 1);
		dummy::dtor = 0;
	}
	BOOST_TEST(dummy::dtor == 3);

	{
		any_t a1 = dummy{};
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		any_t a2 = std::move(a1);
		BOOST_TEST(dummy::copy        == 0);
		BOOST_TEST(dummy::copy_assign == 0);
		BOOST_TEST(dummy::move        == 1);
		BOOST_TEST(dummy::move_assign == 0);
		BOOST_TEST(dummy::dtor        == 0);
	}
	BOOST_TEST(dummy::dtor == 2);

	{
		dummy d{};
		any_t a1 = d;
		any_t a2 = d;
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		a1 = std::move(a2);
		BOOST_TEST(dummy::copy        == 0);
		BOOST_TEST(dummy::copy_assign == 0);
		BOOST_TEST(dummy::move        == 1);
		BOOST_TEST(dummy::move_assign == 0);
		BOOST_TEST(dummy::dtor        == 1);
		dummy::dtor = 0;
	}
	BOOST_TEST(dummy::dtor == 3);

}

BOOST_AUTO_TEST_CASE(test_any_without_move)
{
	using any_t = any::base_any<16, 8, any::iface::copy>;
	{
		dummy d{};
		any_t a1 = d;
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		any_t a2 = a1;
		BOOST_TEST(dummy::copy        == 1);
		BOOST_TEST(dummy::copy_assign == 0);
		BOOST_TEST(dummy::move        == 0);
		BOOST_TEST(dummy::move_assign == 0);
		BOOST_TEST(dummy::dtor        == 0);
	}
	BOOST_TEST(dummy::dtor == 3);

	{
		dummy d{};
		any_t a1 = d;
		any_t a2 = d;
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		a1 = a2;
		BOOST_TEST(dummy::copy        == 1);
		BOOST_TEST(dummy::copy_assign == 0);
		BOOST_TEST(dummy::move        == 0);
		BOOST_TEST(dummy::move_assign == 0);
		BOOST_TEST(dummy::dtor        == 1);
		dummy::dtor = 0;
	}
	BOOST_TEST(dummy::dtor == 3);

	{
		any_t a1 = dummy{};
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		any_t a2 = std::move(a1);
		BOOST_TEST(dummy::copy        == 1);
		BOOST_TEST(dummy::copy_assign == 0);
		BOOST_TEST(dummy::move        == 0);
		BOOST_TEST(dummy::move_assign == 0);
		BOOST_TEST(dummy::dtor        == 0);
	}
	BOOST_TEST(dummy::dtor == 2);

	{
		dummy d{};
		any_t a1 = d;
		any_t a2 = d;
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		a1 = std::move(a2);
		BOOST_TEST(dummy::copy        == 1);
		BOOST_TEST(dummy::copy_assign == 0);
		BOOST_TEST(dummy::move        == 0);
		BOOST_TEST(dummy::move_assign == 0);
		BOOST_TEST(dummy::dtor        == 1);
		dummy::dtor = 0;
	}
	BOOST_TEST(dummy::dtor == 3);

}

BOOST_AUTO_TEST_CASE(test_any_without_copy)
{
	using any_t = any::base_any<16, 8, any::iface::move>;
	{
		any_t a1 = dummy{};
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		any_t a2 = std::move(a1);
		BOOST_TEST(dummy::copy        == 0);
		BOOST_TEST(dummy::copy_assign == 0);
		BOOST_TEST(dummy::move        == 1);
		BOOST_TEST(dummy::move_assign == 0);
		BOOST_TEST(dummy::dtor        == 0);
	}
	BOOST_TEST(dummy::dtor == 2);

	{
		dummy d{};
		any_t a1 = d;
		any_t a2 = d;
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		a1 = std::move(a2);
		BOOST_TEST(dummy::copy        == 0);
		BOOST_TEST(dummy::copy_assign == 0);
		BOOST_TEST(dummy::move        == 1);
		BOOST_TEST(dummy::move_assign == 0);
		BOOST_TEST(dummy::dtor        == 1);
		dummy::dtor = 0;
	}
	BOOST_TEST(dummy::dtor == 3);

}

struct myinterface
{
	using signature_t = double(any::iface::placeholder const&, double);

	template<typename T>
	static double invoke(T const& object, double number)
	{
		return object + number;
	}
};

BOOST_AUTO_TEST_CASE(test_any_custom_interface)
{
	using any_t = any::base_any<16, 8, any::iface::copy, any::iface::move, myinterface>;

	any_t a = 42;
	auto result = a.call<myinterface>(0.5);
	BOOST_TEST(result == 42.5);
	a = 41.5f;
	result = a.call<myinterface>(0.5);
	BOOST_TEST(result == 42.0);

	result = any::call<myinterface>(a, 8.5);
	BOOST_TEST(result == 50.0);
}

BOOST_AUTO_TEST_CASE(test_const_any_custom_interface)
{
	using any_t = any::base_any<16, 8, any::iface::copy, any::iface::move, myinterface>;

	const any_t a1 = 42;
	auto result = a1.call<myinterface>(0.5);
	BOOST_TEST(result == 42.5);

	result = any::call<myinterface>(a1, 8.0);
	BOOST_TEST(result == 50.0);
}

BOOST_AUTO_TEST_CASE(test_copy_ctr_empty)
{
	using any_t = any::base_any<16, 8, any::iface::move, any::iface::copy>;

	any_t a;
	any_t b(a);

	BOOST_TEST(a.has_value() == false);
	BOOST_TEST(b.has_value() == false);
}

BOOST_AUTO_TEST_CASE(test_copy_assign_empty)
{
	using any_t = any::base_any<16, 8, any::iface::move, any::iface::copy>;

	any_t a;
	any_t b;
	b = a;

	BOOST_TEST(a.has_value() == false);
	BOOST_TEST(b.has_value() == false);
}

BOOST_AUTO_TEST_CASE(test_move_ctr_empty)
{
	using any_t = any::base_any<16, 8, any::iface::move, any::iface::copy>;

	any_t a;
	any_t b(std::move(a));

	BOOST_TEST(a.has_value() == false);
	BOOST_TEST(b.has_value() == false);
}

BOOST_AUTO_TEST_CASE(test_move_assign_empty)
{
	using any_t = any::base_any<16, 8, any::iface::move, any::iface::copy>;

	any_t a;
	any_t b;
	b = std::move(a);

	BOOST_TEST(a.has_value() == false);
	BOOST_TEST(b.has_value() == false);
}
