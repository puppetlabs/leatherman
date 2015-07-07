/**
 * @file
 * Declares the base class for scoped resources.
 */
#pragma once

#include <functional>

namespace leatherman { namespace util {
    /**
     * Simple class that is used for the RAII pattern.
     * Used to scope a resource.  When it goes out of scope, a deleter
     * function is called to delete the resource.
     * This type can be moved but cannot be copied.
     * @tparam T The type of resource being scoped.
    */
    template<typename T>
    struct ScopedResource
    {
        /**
         * Constructs an uninitialized ScopedResource.
         * Can be initialized via move assignment.
         */
        ScopedResource() :
            _resource(),
            _deleter(nullptr)
        {
        }

        /**
         * Constructs a ScopedResource.
         * Takes ownership of the given resource.
         * @param resource The resource to scope.
         * @param deleter The function to call when the resource goes out of scope.
         */
        ScopedResource(T resource, std::function<void(T&)> deleter) :
            _resource(std::move(resource)),
            _deleter(deleter)
        {
        }

        /**
         * Prevents the ScopedResource from being copied.
         */
        explicit ScopedResource(ScopedResource<T> const&) = delete;
        /**
         * Prevents the ScopedResource from being copied.
         * @returns Returns this ScopedResource.
         */
        ScopedResource& operator=(ScopedResource<T> const&) = delete;
        /**
         * Moves the given ScopedResource into this ScopedResource.
         * @param other The ScopedResource to move into this ScopedResource.
         */
        ScopedResource(ScopedResource<T>&& other)
        {
            *this = std::move(other);
        }

        /**
         * Moves the given ScopedResource into this ScopedResource.
         * @param other The ScopedResource to move into this ScopedResource.
         * @return Returns this ScopedResource.
         */
        ScopedResource& operator=(ScopedResource<T>&& other)
        {
            release();
            _resource = std::move(other._resource);
            _deleter = std::move(other._deleter);

            // Ensure the deleter is in a known "empty" state; we can't rely on default move semantics for that
            other._deleter = nullptr;
            return *this;
        }

        /**
         * Destructs a ScopedResource.
         */
        ~ScopedResource()
        {
            release();
        }

        /**
         * Implicitly casts to T&.
         * @return Returns reference-to-T.
         */
        operator T&()
        {
            return _resource;
        }

        /**
         * Implicitly casts to T const&.
         * @return Returns const-reference-to-T.
         */
        operator T const&() const
        {
            return _resource;
        }

        /**
         * Releases the resource before destruction.
         */
        void release()
        {
            if (_deleter) {
                _deleter(_resource);
                _deleter = std::function<void(T&)>();
            }
        }

     protected:
        /**
         * Stores the resource being scoped.
         */
        T _resource;
        /**
         * Stores the function to call when the resource goes out of scope.
         */
        std::function<void(T&)> _deleter;

     private:
        void* operator new(size_t) = delete;
        void operator delete(void*) = delete;
        void* operator new[](size_t) = delete;
        void operator delete[](void* ptr) = delete;
    };

}}  // namespace leatherman::util
