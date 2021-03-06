#ifndef EXT_ANY_HEADER
#define EXT_ANY_HEADER

#include <cassert>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

#ifndef EXT_NO_RTTI
#include <typeinfo>
#endif

namespace ext
{
	namespace iface
	{
		struct placeholder
		{ };

		/// destructor interface definition
		/**
			This interface definition is implicitly included in all any-objects.
			\note This is a special interface and is therefore incomplete.
			      See documentation for how to implement custom interfaces.
		*/
		struct destroy
		{
			using signature_t = void(placeholder&);
		};

		/// copy constructor interface definition
		/**
			Use this interface to require objects to be copy-constructable
			\note This is a special interface and is therefore incomplete.
			      See documentation for how to implement custom interfaces.
		*/
		struct copy
		{
			using signature_t = void(placeholder const&, char*);
		};

		/// move constructor interface definition
		/**
			Use this interface to require objects to be move-constructable
			\note This is a special interface and is therefore incomplete.
			      See documentation for how to implement custom interfaces.
		*/
		struct move
		{
			using signature_t = void(placeholder&, char*);
		};

#ifndef EXT_NO_RTTI
		/// type information interface definition
		struct type_info
		{
			using signature_t = std::type_info const& ();
		};
#endif

	} // namespace iface

	// forward declaration
	template<std::size_t Size, std::size_t Alignment, typename... Interfaces> class base_any;

	template<typename>
	struct is_any : std::false_type
	{ };

	template<std::size_t Size, std::size_t Alignment, typename... Interfaces>
	struct is_any<base_any<Size, Alignment, Interfaces...>> : std::true_type
	{ };

	template<class T>
	inline constexpr bool is_any_v = is_any<T>::value;

	namespace _any_detail
	{

		template<typename T>
		using remove_cv_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

		/// interface function dispatcher
		template<typename Interface, typename Signature>
		struct dispatch_impl;

		/// interface function dispatcher for non-const objects
		template<typename Interface, typename Return, typename... Params>
		struct dispatch_impl<Interface, Return(iface::placeholder&, Params...)>
		{
			using function_t = Return(*)(char*, Params...);

			/// converts `data` into `T` and calls the given interface function with its `object` member
			template<typename T>
			static Return invoke_interface(char* data, Params... params)
			{
				return Interface::template invoke(*reinterpret_cast<T*>(data), std::forward<Params>(params)...);
			}
		};

		/// interface function dispatcher for const objects
		template<typename Interface, typename Return, typename... Params>
		struct dispatch_impl<Interface, Return(iface::placeholder const&, Params...)>
		{
			using function_t = Return(*)(char const*, Params...);

			/// converts `data` into `T const*` and calls the given interface function with its `object` member
			template<typename T>
			static Return invoke_interface(char const* data, Params... params)
			{
				return Interface::template invoke(*reinterpret_cast<T const*>(data), std::forward<Params>(params)...);
			}
		};

		/// interface function dispatcher for `iface::destroy`
		template<>
		struct dispatch_impl<iface::destroy, void(iface::placeholder&)>
		{
			using function_t = void(*)(char*);

			template<typename T>
			static void invoke_interface(char* data)
			{
				reinterpret_cast<T*>(data)->~T();
			}
		};

		/// interface function dispatcher for `iface::copy`
		template<>
		struct dispatch_impl<iface::copy, void(iface::placeholder const&, char*)>
		{
			using function_t = void(*)(char const*, char*);

			template<typename T>
			static void invoke_interface(char const* data, char* target)
			{
				new(target) T(*reinterpret_cast<T const*>(data));
			}
		};

		/// interface function dispatcher for `iface::move`
		template<>
		struct dispatch_impl<iface::move, void(iface::placeholder&, char*)>
		{
			using function_t = void(*)(char*, char*);

			template<typename T>
			static void invoke_interface(char* data, char* target)
			{
				new(target) T(std::move(*reinterpret_cast<T*>(data)));
			}
		};

#ifndef EXT_NO_RTTI
		/// interface function dispatcher for `iface::type_info`
		template<>
		struct dispatch_impl<iface::type_info, std::type_info const&()>
		{
			using function_t = std::type_info const& (*)();

			template<typename T>
			static std::type_info const& invoke_interface()
			{
				return typeid(_any_detail::remove_cv_ref_t<T>);
			}
		};
#endif


		/// interface function dispatcher
		template<typename Interface>
		struct dispatch : dispatch_impl<Interface, typename Interface::signature_t>
		{ };

		/// function table entry holding a function pointer for the given interface
		template<typename Interface>
		struct table_entry
		{
			typename dispatch<Interface>::function_t function;
		};

		/// function table for custom interfaces
		template<typename... Interfaces>
		struct fn_table : table_entry<Interfaces>...
		{
			constexpr fn_table(typename dispatch<Interfaces>::function_t... fn_ptrs)
				: table_entry<Interfaces>{fn_ptrs}...
			{ }
		};

		/// function table instance for given T and interfaces
		template<typename T, typename... Interfaces>
		fn_table<
			iface::destroy,
#ifndef EXT_NO_RTTI
			iface::type_info,
#endif
			Interfaces...
		> constexpr function_table{
			dispatch<iface::destroy>::invoke_interface<T>,
#ifndef EXT_NO_RTTI
			dispatch<iface::type_info>::invoke_interface<T>,
#endif
			dispatch<Interfaces>::template invoke_interface<T>...
		};

		/// returns a unique integer, identifying the type and its interfaces associated with given vtable
		template<typename Ptr>
		std::uintptr_t typeid_by_vtable(Ptr* vtable_ptr)
		{
			return reinterpret_cast<std::uintptr_t>(vtable_ptr);
		}

		/// returns a unique integer for given type and its interfaces
		template<typename T, typename... Interfaces>
		std::uintptr_t typeid_by_type()
		{
			return reinterpret_cast<std::uintptr_t>(&function_table<T, Interfaces...>);
		}
	} // namespace _any_detail

	/// any-object, which can carry any object satisfying all given interfaces
	/**
		\code{.cpp}
		\endcode
	*/
	template<std::size_t Size, std::size_t Alignment, typename... Interfaces>
	class alignas(Alignment) base_any
	{
	public:
		constexpr static std::size_t size = Size;
		constexpr static std::size_t alignment = Alignment;

		template<typename OtherType, std::size_t OtherSize, std::size_t OtherAlignment, typename... OtherInterface>
		friend OtherType& any_cast(base_any<OtherSize, OtherAlignment, OtherInterface...>& a);

		template<typename OtherType, std::size_t OtherSize, std::size_t OtherAlignment, typename... OtherInterface>
		friend OtherType const& any_cast(base_any<OtherSize, OtherAlignment, OtherInterface...> const& a);

		template<typename OtherType, std::size_t OtherSize, std::size_t OtherAlignment, typename... OtherInterface>
		friend bool valid_cast(base_any<OtherSize, OtherAlignment, OtherInterface...>& a);

		template<typename OtherType, std::size_t OtherSize, std::size_t OtherAlignment, typename... OtherInterface>
		friend bool valid_cast(base_any<OtherSize, OtherAlignment, OtherInterface...> const& a);

		~base_any()
		{
			destroy();
		}

		base_any()
			: vtable(nullptr)
		{ }

		template<
			typename T,
			typename = std::enable_if_t<!std::is_same<std::decay_t<T>, base_any>::value>
		>
		base_any(T&& object)
			: vtable(&_any_detail::function_table<std::decay_t<T>, Interfaces...>)
		{
			static_assert(sizeof(std::decay_t<T>) <= size, "given object does not fit into this any-object");
			static_assert(alignof(std::decay_t<T>) <= alignment, "given object requires a stricter alignment");
			new(data) std::decay_t<T>(std::forward<T>(object));
		}

		template<
			typename T,
			typename = std::enable_if_t<!std::is_same<std::decay_t<T>, base_any>::value>
		>
		base_any& operator=(T&& object)
		{
			static_assert(sizeof(std::decay_t<T>) <= size, "given object does not fit into this any-object");
			static_assert(alignof(std::decay_t<T>) <= alignment, "given object requires a stricter alignment");
			destroy();
			vtable = &_any_detail::function_table<std::decay_t<T>, Interfaces...>;
			new(data) std::decay_t<T>(std::forward<T>(object));
			return *this;
		}

		base_any(base_any const& other)
			: vtable(other.vtable)
		{
			static_assert(std::is_base_of<_any_detail::table_entry<iface::copy>, _any_detail::fn_table<Interfaces...>>::value,
				"this any-object has no interface for copy construction");

			assert(this != &other && "ill formed initialization");
			if(other.has_value())
				other.interface<iface::copy>().function(other.data, data);
		}

		base_any(base_any&& other)
			: vtable(other.vtable)
		{
			static_assert(
				std::is_base_of<_any_detail::table_entry<iface::move>, _any_detail::fn_table<Interfaces...>>::value
				|| std::is_base_of<_any_detail::table_entry<iface::copy>, _any_detail::fn_table<Interfaces...>>::value,
				"this any-object has neither an interface for move construction nor an interface for copy construction");

			if(other.has_value())
			{
				assert(this != &other && "ill formed initialization");
				if constexpr(std::is_base_of<_any_detail::table_entry<iface::move>, _any_detail::fn_table<Interfaces...>>::value)
					other.interface<iface::move>().function(other.data, data);
				else if(std::is_base_of<_any_detail::table_entry<iface::copy>, _any_detail::fn_table<Interfaces...>>::value)
					other.interface<iface::copy>().function(other.data, data); // fall back to copy construction
			}
			else
				vtable = nullptr;
		}

		base_any& operator= (base_any const& other)
		{
			static_assert(std::is_base_of<_any_detail::table_entry<iface::copy>, _any_detail::fn_table<Interfaces...>>::value,
				"this any-object has no interface for copy construction");

			if(this == &other)
				return *this;

			destroy();
			if(other.has_value())
				other.interface<iface::copy>().function(other.data, data);
			vtable = other.vtable;
			return *this;
		}

		base_any& operator= (base_any&& other)
		{
			static_assert(
				std::is_base_of<_any_detail::table_entry<iface::move>, _any_detail::fn_table<Interfaces...>>::value
				|| std::is_base_of<_any_detail::table_entry<iface::copy>, _any_detail::fn_table<Interfaces...>>::value,
				"this any-object has neither an interface for move construction nor an interface for copy construction");

			if(this == &other)
				return *this;

			destroy();
			if(other.has_value())
			{
				if constexpr(std::is_base_of<_any_detail::table_entry<iface::move>, _any_detail::fn_table<Interfaces...>>::value)
					other.interface<iface::move>().function(other.data, data);
				else if(std::is_base_of<_any_detail::table_entry<iface::copy>, _any_detail::fn_table<Interfaces...>>::value)
					other.interface<iface::copy>().function(other.data, data); // fall back to copy construction

			}
			vtable = other.vtable;
			return *this;
		}

		/// calls the given interface function of the inner object
		template<typename Interface, typename... Args>
		decltype(auto) call(Args&&... args)
		{
			static_assert(std::is_base_of<_any_detail::table_entry<Interface>, _any_detail::fn_table<Interfaces...>>::value,
				"this any-object does not support given interface");

			return interface<Interface>().function(data, std::forward<Args>(args)...);
		}

		/// calls the given interface function of the inner object
		template<typename Interface, typename... Args>
		decltype(auto) call(Args&&... args) const
		{
			static_assert(std::is_base_of<_any_detail::table_entry<Interface>, _any_detail::fn_table<Interfaces...>>::value,
				"this any-object does not support given interface");

			return interface<Interface>().function(data, std::forward<Args>(args)...);
		}

		/// returns true if this any contains a value, false otherwise
		bool has_value() const
		{
			return vtable != nullptr;
		}

#ifndef EXT_NO_RTTI
		auto type() const -> std::type_info const&
		{
			if(has_value())
				return interface<iface::type_info>().function();
			else
				return typeid(void);
		}
#endif

		/// destroys the inner object (has_value() returns false afterwards)
		void reset()
		{
			destroy();
			vtable = nullptr;
		}

	private:
		template<typename Interface>
		decltype(auto) interface() const
		{
			assert(has_value());
			return *static_cast<_any_detail::table_entry<Interface> const*>(vtable);
		}

		void destroy()
		{
			if(has_value())
				interface<iface::destroy>().function(data);
		}

	private:
		char data[size];
		_any_detail::fn_table<
			iface::destroy,
#ifndef EXT_NO_RTTI
			iface::type_info,
#endif
			Interfaces...
		> const* vtable;
	};

	/// free-standing-function equivalent to base_any::has_value()
	template<std::size_t Size, std::size_t Alignment, typename... Interfaces>
	bool has_value(base_any<Size, Alignment, Interfaces...> const& a)
	{
		return a.has_value();
	}

	/// returns true if the given cast is valid
	template<typename T, std::size_t Size, std::size_t Alignment, typename... Interfaces>
	bool valid_cast(base_any<Size, Alignment, Interfaces...>& a)
	{
		return _any_detail::typeid_by_vtable(a.vtable) == _any_detail::typeid_by_type<T, Interfaces...>()
#ifndef EXT_NO_RTTI
		       || a.type() == typeid(T)
#endif
		;
	}

	/// returns true if the given cast is valid
	template<typename T, std::size_t Size, std::size_t Alignment, typename... Interfaces>
	bool valid_cast(base_any<Size, Alignment, Interfaces...> const& a)
	{
		return _any_detail::typeid_by_vtable(a.vtable) == _any_detail::typeid_by_type<T, Interfaces...>()
#ifndef EXT_NO_RTTI
		       || a.type() == typeid(T)
#endif
		;
	}

	/// returns a reference to the given type
	/**
		\note If the given any does not contain the given type, using the returned reference is undefined behavior
		\see valid_cast
	*/
	template<typename T, std::size_t Size, std::size_t Alignment, typename... Interfaces>
	T& any_cast(base_any<Size, Alignment, Interfaces...>& a)
	{
		assert(valid_cast<T>(a) && "any_cast: any-object does not contain given type");
		return *reinterpret_cast<T*>(a.data);
	}

	/// returns a reference to the given type
	/**
		\note If the given any does not contain the given type, using the returned reference is undefined behavior
		\see valid_cast
	*/
	template<typename T, std::size_t Size, std::size_t Alignment, typename... Interfaces>
	T const& any_cast(base_any<Size, Alignment, Interfaces...> const& a)
	{
		assert(valid_cast<T>(a) && "any_cast: any-object does not contain given type");
		return *reinterpret_cast<T const*>(a.data);
	}

	/// calls the given interface function of the any's inner object
	/**
		\note Calling this function on an empty any is undefined behavior
		\see base_any::has_value
	*/
	template<typename Interface, std::size_t Size, std::size_t Alignment, typename... Interfaces, typename... Args>
	decltype(auto) call(base_any<Size, Alignment, Interfaces...>& a, Args&&... args)
	{
		return a.template call<Interface>(std::forward<Args>(args)...);
	}

	/// calls the given interface function of the any's inner object
	/**
		\note Calling this function on an empty any is undefined behavior
		\see base_any::has_value
	*/
	template<typename Interface, std::size_t Size, std::size_t Alignment, typename... Interfaces, typename... Args>
	decltype(auto) call(base_any<Size, Alignment, Interfaces...> const& a, Args&&... args)
	{
		return a.template call<Interface>(std::forward<Args>(args)...);
	}

	template<std::size_t Size, std::size_t Alignment = 8>
	using any = base_any<Size, Alignment, iface::copy>;
} // namespace any

#endif // EXT_ANY_HEADER
