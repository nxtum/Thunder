/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 Metrological
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#ifndef MODULE_NAME
#include "../Module.h"
#endif

#include <core/core.h>

namespace Thunder {
namespace Tests {
namespace Core {

    class DataFile: public ::Thunder::Core::DataElementFile
    {
    public:
        DataFile() = delete;

        DataFile(::Thunder::Core::File& file)
            : ::Thunder::Core::DataElementFile(file, ::Thunder::Core::File::USER_READ)
        {
        }

        DataFile(string fileName, uint32_t type, uint32_t size)
            : ::Thunder::Core::DataElementFile(fileName, type, size)
        {
        }

        void MemoryMap()
        {
            Reallocation(54);
            ReopenMemoryMappedFile();
        }
    };

    TEST(test_datafile, simple_test)
    {
        const string fileName = "dataFile.txt";
        const string message = ">echo 'DataElement file checking......'";
        const string buffer = message + fileName;

#ifdef __POSIX__
        errno = 0;
#endif

        ::Thunder::Core::File file(fileName);

        ASSERT_FALSE(file.Exists());

         // Always for a non-existing file
#ifdef __WINDOWS__
        EXPECT_EQ(file.ErrorCode(), static_cast<uint32_t>(ERROR_FILE_NOT_FOUND));
#endif
#ifdef __POSIX__
        EXPECT_EQ(file.ErrorCode(), static_cast<uint32_t>(ENOENT));
#endif

        ASSERT_TRUE(file.Create(true));
        EXPECT_TRUE(file.IsOpen());
        EXPECT_STREQ(file.Name().c_str(), fileName.c_str());

#ifdef __POSIX__
        errno = 0;
#endif

        DataFile obj1(file);
#ifdef __WINDOWS__
        EXPECT_EQ(obj1.ErrorCode(), static_cast<uint32_t>(ERROR_SUCCESS));
#endif
#ifdef __POSIX__
        EXPECT_EQ(obj1.ErrorCode(), static_cast<uint32_t>(0));
#endif

        DataFile object(fileName, 1, 10);
        DataFile obj2(fileName, 1, 50);

        obj1.Sync();
        obj2.MemoryMap();

        const string& name = obj1.Name();

        EXPECT_EQ(name.c_str(), fileName);
        EXPECT_EQ(obj2.IsValid(), true);

        const ::Thunder::Core::File& obj1File = obj1.Storage();
        EXPECT_TRUE(obj1File.IsOpen());
        EXPECT_STREQ(obj1File.FileName().c_str(), file.FileName().c_str());

        obj1.ReloadFileInfo();
        obj1.MemoryMap();

        EXPECT_TRUE(file.Destroy());
    }

} // Core
} // Tests
} // Thunder
