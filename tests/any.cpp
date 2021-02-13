#include <gtest/gtest.h>
#include <ext/any.hpp>

TEST(is_any, static_assert)
{
	static_assert(!ext::is_any<int>::value);
	static_assert( ext::is_any<ext::base_any<16, 8>>::value);
}

TEST(types, int)
{
	using any_t = ext::any<16, 8>;

	any_t a = 42;
	EXPECT_EQ(ext::valid_cast<int>(a), true);

	decltype(auto) result = ext::any_cast<int>(a);
	static_assert(std::is_same<decltype(result), int&>::value);
	EXPECT_EQ(result, 42);

	result = 170;
	EXPECT_EQ(ext::any_cast<int>(a), 170);

	any_t const& b = a;
	EXPECT_EQ(ext::valid_cast<int>(b), true);
	EXPECT_EQ(ext::any_cast<int>(b), 170);

	decltype(auto) result2 = ext::any_cast<int>(b);
	static_assert(std::is_same<decltype(result2), int const&>::value);
	EXPECT_EQ(result2, 170);

    result = 682;
	EXPECT_EQ(result2, 682);
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


TEST(any_interface, copy_move)
{
	using any_t = ext::base_any<16, 8, ext::iface::copy, ext::iface::move>;
	{
		dummy d{};
		any_t a1 = d;
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		any_t a2 = a1;
		EXPECT_EQ(dummy::copy       , 1);
		EXPECT_EQ(dummy::copy_assign, 0);
		EXPECT_EQ(dummy::move       , 0);
		EXPECT_EQ(dummy::move_assign, 0);
		EXPECT_EQ(dummy::dtor       , 0);
	}
	EXPECT_EQ(dummy::dtor, 3);

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
		EXPECT_EQ(dummy::copy       , 1);
		EXPECT_EQ(dummy::copy_assign, 0);
		EXPECT_EQ(dummy::move       , 0);
		EXPECT_EQ(dummy::move_assign, 0);
		EXPECT_EQ(dummy::dtor       , 1);
		dummy::dtor = 0;
	}
	EXPECT_EQ(dummy::dtor, 3);

	{
		any_t a1 = dummy{};
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		any_t a2 = std::move(a1);
		EXPECT_EQ(dummy::copy       , 0);
		EXPECT_EQ(dummy::copy_assign, 0);
		EXPECT_EQ(dummy::move       , 1);
		EXPECT_EQ(dummy::move_assign, 0);
		EXPECT_EQ(dummy::dtor       , 0);
	}
	EXPECT_EQ(dummy::dtor, 2);

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
		EXPECT_EQ(dummy::copy       , 0);
		EXPECT_EQ(dummy::copy_assign, 0);
		EXPECT_EQ(dummy::move       , 1);
		EXPECT_EQ(dummy::move_assign, 0);
		EXPECT_EQ(dummy::dtor       , 1);
		dummy::dtor = 0;
	}
	EXPECT_EQ(dummy::dtor, 3);

}

TEST(any_interface, without_move)
{
	using any_t = ext::base_any<16, 8, ext::iface::copy>;
	{
		dummy d{};
		any_t a1 = d;
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		any_t a2 = a1;
		EXPECT_EQ(dummy::copy       , 1);
		EXPECT_EQ(dummy::copy_assign, 0);
		EXPECT_EQ(dummy::move       , 0);
		EXPECT_EQ(dummy::move_assign, 0);
		EXPECT_EQ(dummy::dtor       , 0);
	}
	EXPECT_EQ(dummy::dtor, 3);

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
		EXPECT_EQ(dummy::copy       , 1);
		EXPECT_EQ(dummy::copy_assign, 0);
		EXPECT_EQ(dummy::move       , 0);
		EXPECT_EQ(dummy::move_assign, 0);
		EXPECT_EQ(dummy::dtor       , 1);
		dummy::dtor = 0;
	}
	EXPECT_EQ(dummy::dtor, 3);

	{
		any_t a1 = dummy{};
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		any_t a2 = std::move(a1);
		EXPECT_EQ(dummy::copy       , 1);
		EXPECT_EQ(dummy::copy_assign, 0);
		EXPECT_EQ(dummy::move       , 0);
		EXPECT_EQ(dummy::move_assign, 0);
		EXPECT_EQ(dummy::dtor       , 0);
	}
	EXPECT_EQ(dummy::dtor, 2);

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
		EXPECT_EQ(dummy::copy       , 1);
		EXPECT_EQ(dummy::copy_assign, 0);
		EXPECT_EQ(dummy::move       , 0);
		EXPECT_EQ(dummy::move_assign, 0);
		EXPECT_EQ(dummy::dtor       , 1);
		dummy::dtor = 0;
	}
	EXPECT_EQ(dummy::dtor, 3);

}

TEST(any_interface, without_copy)
{
	using any_t = ext::base_any<16, 8, ext::iface::move>;
	{
		any_t a1 = dummy{};
		dummy::copy        = 0;
		dummy::copy_assign = 0;
		dummy::move        = 0;
		dummy::move_assign = 0;
		dummy::dtor        = 0;
		any_t a2 = std::move(a1);
		EXPECT_EQ(dummy::copy       , 0);
		EXPECT_EQ(dummy::copy_assign, 0);
		EXPECT_EQ(dummy::move       , 1);
		EXPECT_EQ(dummy::move_assign, 0);
		EXPECT_EQ(dummy::dtor       , 0);
	}
	EXPECT_EQ(dummy::dtor, 2);

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
		EXPECT_EQ(dummy::copy       , 0);
		EXPECT_EQ(dummy::copy_assign, 0);
		EXPECT_EQ(dummy::move       , 1);
		EXPECT_EQ(dummy::move_assign, 0);
		EXPECT_EQ(dummy::dtor       , 1);
		dummy::dtor = 0;
	}
	EXPECT_EQ(dummy::dtor, 3);

}

struct myinterface
{
	using signature_t = double(ext::iface::placeholder const&, double);

	template<typename T>
	static double invoke(T const& object, double number)
	{
		return object + number;
	}
};

TEST(any_interface, custom)
{
	using any_t = ext::base_any<16, 8, ext::iface::copy, ext::iface::move, myinterface>;

	any_t a = 42;
	auto result = a.call<myinterface>(0.5);
	EXPECT_EQ(result, 42.5);
	a = 41.5f;
	result = a.call<myinterface>(0.5);
	EXPECT_EQ(result, 42.0);

	result = ext::call<myinterface>(a, 8.5);
	EXPECT_EQ(result, 50.0);
}

TEST(any_interface, const_custom_interface)
{
	using any_t = ext::base_any<16, 8, ext::iface::copy, ext::iface::move, myinterface>;

	const any_t a1 = 42;
	auto result = a1.call<myinterface>(0.5);
	EXPECT_EQ(result, 42.5);

	result = ext::call<myinterface>(a1, 8.0);
	EXPECT_EQ(result, 50.0);
}

TEST(any_special_member_fn, test_has_value)
{
	using any_t = ext::base_any<16, 8, ext::iface::move, ext::iface::copy>;

	any_t a;

	EXPECT_EQ(a.has_value(), false);
	EXPECT_EQ(has_value(a), false);

	a = 42;

	EXPECT_EQ(a.has_value(), true);
	EXPECT_EQ(has_value(a), true);

	a.reset();

	EXPECT_EQ(a.has_value(), false);
	EXPECT_EQ(has_value(a), false);
}

TEST(any_special_member_fn, copy_ctr_empty)
{
	using any_t = ext::base_any<16, 8, ext::iface::move, ext::iface::copy>;

	any_t a;
	any_t b(a);

	EXPECT_EQ(a.has_value(), false);
	EXPECT_EQ(b.has_value(), false);
}

TEST(any_special_member_fn, copy_assign_empty)
{
	using any_t = ext::base_any<16, 8, ext::iface::move, ext::iface::copy>;

	any_t a;
	any_t b;
	b = a;

	EXPECT_EQ(a.has_value(), false);
	EXPECT_EQ(b.has_value(), false);
}

TEST(any_special_member_fn, move_ctr_empty)
{
	using any_t = ext::base_any<16, 8, ext::iface::move, ext::iface::copy>;

	any_t a;
	any_t b(std::move(a));

	EXPECT_EQ(a.has_value(), false);
	EXPECT_EQ(b.has_value(), false);
}

TEST(any_special_member_fn, move_assign_empty)
{
	using any_t = ext::base_any<16, 8, ext::iface::move, ext::iface::copy>;

	any_t a;
	any_t b;
	b = std::move(a);

	EXPECT_EQ(a.has_value(), false);
	EXPECT_EQ(b.has_value(), false);
}

TEST(any_special_member_fn, any_in_any)
{
	using big_any_t = ext::base_any<16, 8, ext::iface::copy>;
	using small_any_t = ext::base_any<8, 8, ext::iface::copy>;

	big_any_t b;
	small_any_t s;

	s = 42;
	EXPECT_EQ(s.has_value(), true);
	EXPECT_EQ(ext::valid_cast<int>(s), true);
	EXPECT_EQ(b.has_value(), false);

	b = s;
	EXPECT_EQ(b.has_value(), true);
	EXPECT_EQ(ext::valid_cast<small_any_t>(b), true);
	decltype(auto) result = ext::any_cast<small_any_t>(b);
	static_assert(std::is_same_v<decltype(result), small_any_t&>);
	EXPECT_EQ(ext::valid_cast<int>(result), true);
	decltype(auto) result2 = ext::any_cast<int>(result);
	static_assert(std::is_same_v<decltype(result2), int&>);
	EXPECT_EQ(result2, 42);
}
