#include <iterator>
#include <stdexcept>
#include <type_traits>
namespace raine {
    template <class _Tp, size_t _Size>
    struct tweaked_array {
        typedef tweaked_array __self;
        typedef _Tp value_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef value_type* iterator;
        typedef const value_type* const_iterator;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

        _Tp __elems_[_Size];

        void swap(tweaked_array& a) noexcept {
            std::swap_ranges(__elems_,__elems_+_Size, a.__elems_);
        }

        iterator begin() noexcept {
            return iterator(data());
        }

        const_iterator begin() const noexcept {
            return const_iterator(data());
        }

        iterator end() noexcept {
            return iterator(data()+_Size);
        }

        const_iterator end() const noexcept {
            return const_iterator(data()+_Size);
        }

        reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        const_iterator cbegin() noexcept {
            return begin();
        }

        const_iterator cend() noexcept {
            return end();
        }

        const_iterator crbegin() noexcept {
            return rbegin();
        }

        const_iterator crend()  noexcept {
            return rend();
        }

        constexpr size_type size() const noexcept {
            return _Size;
        }

        constexpr size_type max_size() const noexcept {
            return _Size;
        }

        constexpr bool empty() const noexcept { return false; }
        
        reference operator[](size_type n) noexcept {
            return __elems_[n];
        }

        const_reference operator[](size_type n) const noexcept {
            return __elems_[n];
        }

        reference at(size_type n);
        const_reference at(size_type n) const;
        reference front() noexcept { return __elems_[0]; }
        const_reference  front() const noexcept { return __elems_[0]; }
        reference back() noexcept { return __elems_[_Size-1]; }
        const_reference back() const noexcept { return __elems_[_Size-1]; }
        value_type* data() noexcept { return __elems_; }
        const value_type* data() const noexcept { return __elems_; }

    };

    template<class _Tp, size_t _Size>
    typename tweaked_array<_Tp,_Size>::reference
    tweaked_array<_Tp,_Size>::at(size_type n) {
        if (n >= _Size) {
            throw std::out_of_range("array::at");
        }
        return __elems_[n];
    }

    template<class _Tp, size_t _Size>
    typename tweaked_array<_Tp, _Size>::const_reference
    tweaked_array<_Tp,_Size>::at(size_type n) const {
        if (n >= _Size) {
            throw std::out_of_range("array::at");
        }
        return __elems_[n];
    }

    template<class _Tp>
    struct tweaked_array<_Tp,0>
    {
        typedef tweaked_array __self;
        typedef _Tp value_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef value_type* iterator;
        typedef const value_type* const_iterator;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        
        typedef typename std::conditional<std::is_const<_Tp>::value,const char, char>::type _CharType;


        struct _ArrayInStructT { _Tp __data[1]; };
        alignas(_ArrayInStructT) _CharType __elems[sizeof(_ArrayInStructT)];

        void swap(tweaked_array&) noexcept {
            static_assert(!std::is_const<_Tp>::value, "cannot swap zero-sized array of type 'const T'");
        }

        iterator begin() noexcept {
            return iterator(data());
        }

        const_iterator begin() const noexcept {
            return const_iterator(data());
        }

        iterator end() noexcept {
            return iterator(data());
        }

        const_iterator end() const noexcept {
            return const_iterator(data());
        }

        reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }

        const_reverse_iterator crend() const noexcept {
            return rend();
        }

        constexpr size_type size() const noexcept { return 0; }
        constexpr size_type max_size() const noexcept { return 0; }
        constexpr bool empty() const noexcept { return true; }
        reference operator[](size_type) noexcept {
            // static_assert(false, "cannot call array<T, 0>::operator[] on a zero-sized array");
            exit(1);
        }

        const_reference operator[](size_type) const noexcept {
            // static_assert(false, "cannot call array<T, 0>::operator[] on a zero-sized array");
            exit(1); // never reach
        }

        reference at(size_type) {
            throw std::out_of_range("tweaked_array<T,0>::at");
        }

        const_reference at(size_type) const {
            throw std::out_of_range("tweaked_array<T,0>::at");
        }

        reference front() noexcept {
            // assert(false, "cannot call array<T, 0>::front() on a zero-sized array");
            exit(1);
        }

        const_reference front() const noexcept {
            // assert(false, "cannot call array<T, 0>::front() on a zero-sized array");
            exit(1);
        }

        reference back() noexcept {
            // assert(false, "cannot call array<T, 0>::front() on a zero-sized array");
            exit(1);
        }

        const_reference back() const noexcept {
            // assert(false, "cannot call array<T, 0>::front() on a zero-sized array");
            exit(1);
        }
        
        value_type* data() noexcept {
            return reinterpret_cast<value_type*>(__elems);
        }

        const value_type* data() const noexcept {
            return reinterpret_cast<value_type*>(__elems);
        }
    };

    template <class Tp, size_t Size>
    inline bool operator== (const tweaked_array<Tp,Size>& x, const tweaked_array<Tp,Size>& y) {
        for (int i=0;i<x.size();i++) {
            if (x[i] == y[i]) {
                continue;
            } else {
                return false;
            }
        }
        return true;
    }

    template <class Tp, size_t Size>
    inline bool operator!= (const tweaked_array<Tp,Size>& x, const tweaked_array<Tp,Size>& y) {
        return !(x == y);
    }
};