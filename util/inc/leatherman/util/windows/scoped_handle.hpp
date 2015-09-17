/**
 * @file
 * Declares the scoped HANDLE resource for managing Windows HANDLEs.
 */
#pragma once

#include <leatherman/util/scoped_resource.hpp>
typedef void *HANDLE;

namespace leatherman { namespace util { namespace windows {
    /**
     * Represents a scoped HANDLE for Windows.
     * Automatically closes the HANDLE when it goes out of scope.
    */
    struct scoped_handle : scoped_resource<HANDLE>
    {
        /**
         * Constructs a scoped_handle.
         * @param handle The HANDLE to close when destroyed.
         */
        explicit scoped_handle(HANDLE handle);

        /**
         * Constructs a closed scoped_handle.
         */
        scoped_handle();

     private:
        static void close(HANDLE handle);
    };

}}}  // namespace leatherman::util::windows
