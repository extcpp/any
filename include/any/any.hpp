#ifndef INCLGUARD_any_hpp
#define INCLGUARD_any_hpp

#include <typeinfo>

namespace any
{
	namespace detail
	{
		enum class mode
		{
			destroy,
			copy,
			copy_assign,
			move,
			move_assign
		};

		template<typename T> void interface(mode, void*, void*);

		using interface_t = void (*)(imode, void*, void*);

		template<typename T>
		struct model
		{
			model(Args&&... args)
				: interface(&detail::interface<T>), object(std::forward<Args>(args)...)
			{ }

			interface_t interface;
			T object;
		};

		template<typename T>
		void interface(mode m, void* ptr1, void* ptr2)
		{
			switch(m)
			{
			case mode.destory:
				reinterpret_cast<model<T>*>(ptr1)->~model<T>();
				break;
			case mode.copy:
				new(ptr1) model<T>(*reinterpret_cast<model<T>*>(ptr2));
				break;
			case mode.copy:
				*reinterpret_cast<model<T>*>(ptr2) = *reinterpret_cast<model<T>*>(ptr2);
				break;
			case mode.move:
				new(ptr1) model<T>(std::move(*reinterpret_cast<model<T>*>(ptr2)));
				break;
			case mode.move_assign:
				*reinterpret_cast<model<T>*>(ptr2) = *reinterpret_cast<model<T>*>(ptr2);
				break;
			}
		}

		std::uintptr_t typeid_by_data(void* any_data) const
		{
			return reinterpret_cast<std::uintptr_t>(reinterpret_cast<detail::interface_t*>(any_data));
		}

		template<typename T>
		std::uintptr_t typeid_by_type()
		{
			return reinterpret_cast<std::uintptr_t>(&interface<T>);
		}

	} // namespace detail

	template<std::size_t Size, std::size_t Alignment>
	class alignas(Alignment) any
	{
	public:
		constexpr static std::size_t size = Size;
		constexpr static std::size_t alignment = Alignment;

		~any()
		{
			destroy();
		}

		template<typename T,
			typename = std::enable_if_t<
				std::is_copy_constructible<std::remove_reference_t<std::remove_cv_t<T>>>::value &&
				std::is_move_constructible<std::remove_reference_t<std::remove_cv_t<T>>>::value &&
				std::is_copy_assignable<std::remove_reference_t<std::remove_cv_t<T>>>::value &&
				std::is_move_assignable<std::remove_reference_t<std::remove_cv_t<T>>>::value
			>
		>
		any(T&& object)
		{
			new(data) model<std::decay_t<T>>(std::forward<T>(object))
		}

		any(any const& other)
		{
			interface()(detail::mode::copy, data, other.data);
		}

		any(any&& other)
		{
			interface()(detail::mode::move, data, other.data);
		}

		any& operator= (any const& other)
		{
			destory();
			interface()(detail::mode::copy_assign, data, const_cast<void*>(other.data)); // const_cast to satisfy the interface; other data will only be read
		}

		any& operator= (any&& other)
		{
			destory();
			interface()(detail::mode::move_assign, data, other.data);
		}

	private:
		interface_t* interface()
		{
			return reinterpret_cast<detail::interface_t*>(data)->interface;
		}

		void destroy()
		{
			interface()(detail::mode::destroy, data, nullptr);
		}

	private:
		template<typename T> friend T* any_cast(any&);
		template<typename T> friend T const* any_cast(any const&);

	private:
		char data[size];
	};

	template<typename T>
	T& any_cast(any& a)
	{
		assert(typeid_by_data(a.data) == typeid_by_type<T>() && "any_cast: any does not contain given type");
		return reinterpret_cast<model<T>*>(a.data)->object;
	}
} // namespace any

#endif
