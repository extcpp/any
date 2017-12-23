#ifndef INCLGUARD_any_hpp
#define INCLGUARD_any_hpp

#include <typeinfo>

namespace any
{
	namespace iface
	{
		struct placeholder
		{ };

		using _1 = placeholder;

		struct destroy
		{
			using signature_t = void(placeholder&);
		};

		struct copy
		{
			using signature_t = void(placeholder const&, char*);
		};

		struct move
		{
			using signature_t = void(placeholder&, char*);
		};
	}

	namespace detail
	{
		template<typename Interface>
		struct dispatch;

		template<typename Interface>
		struct table_entry
		{
			typename dispatch<Interface>::function_t function;
		};

		/// function table for custom interfaces
		/**
			\note `iface::destroy` is always part of the interface
		*/
		template<typename... Interfaces>
		struct fn_table : table_entry<Interfaces>...
		{
			fn_table(typename dispatch<Interfaces>::function_t... fn_ptrs)
				: table_entry<Interfaces>{fn_ptrs}...
			{ }
		};

		template<typename T, typename... Interfaces>
		fn_table<iface::destroy, Interfaces...> const*
		function_table();

		template<typename T, typename... Interfaces>
		struct model
		{
			template<typename... Args>
			model(Args&&... args)
				: table(function_table<T, Interfaces...>())
				, object(std::forward<Args>(args)...)
			{ }

			model(model const&) = default;
			model& operator= (model const&) = default;
			model(model&&) = default;
			model& operator= (model&&) = default;

			fn_table<iface::destroy, Interfaces...> const* const table;
			T object;
		};

		template<typename Interface, typename Signature>
		struct dispatch_impl;

		/// interface function dispatcher (helper)
		template<typename Interface, typename Return, typename... Params>
		struct dispatch_impl<Interface, Return(iface::placeholder&, Params...)>
		{
			using function_t = Return(*)(char*, Params...);

			/// converts `data` into `model<T, Interfaces>` and calls the given interface function with its `object` member
			template<typename T, typename... Interfaces>
			static Return invoke(char* data, Params... params)
			{
				return Interface::template invoke<T>(reinterpret_cast<model<T, Interfaces...>*>(data)->object, std::forward<Params>(params)...);
			}
		};

		/// interface function dispatcher (helper)
		template<typename Interface, typename Return, typename... Params>
		struct dispatch_impl<Interface, Return(iface::placeholder const&, Params...)>
		{
			using function_t = Return(*)(char const*, Params...);

			/// converts `data` into `model<T,Interfaces> const*` and calls the given interface function with its `object` member
			template<typename T, typename... Interfaces>
			static Return invoke(char const* data, Params... params)
			{
				return Interface::template invoke<T>(reinterpret_cast<model<T, Interfaces...> const*>(data)->object, std::forward<Params>(params)...);
			}
		};

		/// interface function dispatcher for `iface::destroy`
		template<>
		struct dispatch_impl<iface::destroy, void(iface::placeholder&)>
		{
			using function_t = void(*)(char*);

			template<typename T, typename... Interfaces>
			static void invoke(char* data)
			{
				reinterpret_cast<model<T, Interfaces...>*>(data)->~model<T, Interfaces...>();
			}
		};

		/// interface function dispatcher for `iface::copy`
		template<>
		struct dispatch_impl<iface::copy, void(iface::placeholder const&, char*)>
		{
			using function_t = void(*)(char const*, char*);

			template<typename T, typename... Interfaces>
			static void invoke(char const* data, char* target)
			{
				new(target) detail::model<T, Interfaces...>(reinterpret_cast<model<T, Interfaces...> const*>(data)->object);
			}
		};

		/// interface function dispatcher for `iface::move`
		template<>
		struct dispatch_impl<iface::move, void(iface::placeholder&, char*)>
		{
			using function_t = void(*)(char*, char*);

			template<typename T, typename... Interfaces>
			static void invoke(char* data, char* target)
			{
				new(target) detail::model<T, Interfaces...>(std::move(reinterpret_cast<model<T, Interfaces...>*>(data)->object));
			}
		};

		/// interface function dispatcher
		template<typename Interface>
		struct dispatch : dispatch_impl<Interface, typename Interface::signature_t>
		{ };

		template<typename T, typename... Interfaces>
		fn_table<iface::destroy, Interfaces...> const function_table_impl(dispatch<iface::destroy>::invoke<T, Interfaces...>, dispatch<Interfaces>::template invoke<T, Interfaces...>...);

		template<typename T, typename... Interfaces>
		fn_table<iface::destroy, Interfaces...> const*
		function_table()
		{
			return &function_table_impl<T, Interfaces...>;
		}

		template<typename Ptr>
		std::uintptr_t typeid_by_vtable(Ptr* vtable_ptr)
		{
			return reinterpret_cast<std::uintptr_t>(vtable_ptr);
		}

		template<typename T, typename... Interfaces>
		std::uintptr_t typeid_by_type()
		{
			return reinterpret_cast<std::uintptr_t>(function_table<T, Interfaces...>());
		}

	} // namespace detail

	template<std::size_t Size, std::size_t Alignment, typename... Interfaces>
	class alignas(Alignment) base_any
	{
	public:
		constexpr static std::size_t size = Size;
		constexpr static std::size_t alignment = Alignment;

		template<typename _T, std::size_t _Size, std::size_t _Alignment, typename... _Interfaces>
		friend _T& any_cast(base_any<_Size, _Alignment, _Interfaces...>& a);

		template<typename _T, std::size_t _Size, std::size_t _Alignment, typename... _Interfaces>
		friend bool valid_cast(base_any<_Size, _Alignment, _Interfaces...>& a);

		~base_any()
		{
			destroy();
		}

		template<
			typename T,
			typename = std::enable_if_t<!std::is_same<std::remove_reference_t<std::remove_cv_t<T>>, base_any<Size, Alignment, Interfaces...>>::value>
		>
		base_any(T&& object)
			: vtable(detail::function_table<std::decay_t<T>, Interfaces...>())
		{
			static_assert(sizeof(detail::model<std::decay_t<T>>) <= size, "given object does not fit into this any");
			static_assert(alignof(detail::model<std::decay_t<T>>) <= alignment, "given object requires a greater alignment");
			new(data) detail::model<std::decay_t<T>, Interfaces...>(std::forward<T>(object));
		}

		base_any(base_any const& other)
			: vtable(other.vtable)
		{
			static_assert(std::is_base_of<detail::table_entry<iface::copy>, detail::fn_table<Interfaces...>>::value,
				"this any has no interface for copy construction");

			assert(this != &other && "ill formed initialization");
			other.interface<iface::copy>().function(other.data, data);
		}

		base_any(base_any&& other)
			: vtable(other.vtable)
		{
			static_assert(std::is_base_of<detail::table_entry<iface::move>, detail::fn_table<Interfaces...>>::value,
				"this any has no interface for move construction");

			assert(this != &other && "ill formed initialization");
			other.interface<iface::move>().function(other.data, data);
		}

		base_any& operator= (base_any const& other)
		{
			static_assert(std::is_base_of<detail::table_entry<iface::copy>, detail::fn_table<Interfaces...>>::value,
				"this any has no interface for copy construction");

			if(this == &other)
				return *this;

			destroy();
			other.interface<iface::copy>().function(other.data, data);
			vtable = other.vtable;
			return *this;
		}

		base_any& operator= (base_any&& other)
		{
			static_assert(std::is_base_of<detail::table_entry<iface::move>, detail::fn_table<Interfaces...>>::value,
				"this any has no interface for move construction");

			if(this == &other)
				return *this;

			destroy();
			other.interface<iface::move>().function(other.data, data);
			vtable = other.vtable;
			return *this;
		}

		template<typename Interface, typename... Args>
		auto call(Args&&... args)
		{
			static_assert(std::is_base_of<detail::table_entry<Interface>, detail::fn_table<Interfaces...>>::value,
				"this any does not support given interface");

			return interface<Interface>().function(data, std::forward<Args>(args)...);
		}

		template<typename Interface, typename... Args>
		auto call(Args&&... args) const
		{
			static_assert(std::is_base_of<detail::table_entry<Interface>, detail::fn_table<Interfaces...>>::value,
				"this any does not support given interface");

			return interface<Interface>().function(data, std::forward<Args>(args)...);
		}

	private:
		template<typename Interface>
		auto interface() const
		{
			return *static_cast<detail::table_entry<Interface> const*>(vtable);
		}

		void destroy()
		{
			interface<iface::destroy>().function(data);
		}

	private:
		char data[size];
		detail::fn_table<iface::destroy, Interfaces...> const* vtable;
	};

	template<typename T, std::size_t Size, std::size_t Alignment, typename... Interfaces>
	bool valid_cast(base_any<Size, Alignment, Interfaces...>& a)
	{
		return detail::typeid_by_vtable(a.vtable) == detail::typeid_by_type<T, Interfaces...>();
	}

	template<typename T, std::size_t Size, std::size_t Alignment, typename... Interfaces>
	T& any_cast(base_any<Size, Alignment, Interfaces...>& a)
	{
		assert(valid_cast<T>(a) && "any_cast: any does not contain given type");
		return reinterpret_cast<detail::model<T, Interfaces...>*>(a.data)->object;
	}

	template<std::size_t Size, std::size_t Alignment = 8>
	using any = base_any<Size, Alignment, iface::copy>;
} // namespace any

#endif
