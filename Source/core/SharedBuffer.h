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
 
#ifndef __SHARED_BUFFER_H
#define __SHARED_BUFFER_H

#include <memory> // align

// ---- Include local include files ----
#include "DataElementFile.h"
#include "Module.h"

#ifndef __WINDOWS__
#include <semaphore.h>
#endif

// ---- Referenced classes and types ----

// ---- Helper types and constants ----

namespace Thunder {

namespace Core {
    // Rationale:
    // This class allows to share data over process boundaries. This is a simple version of the
    // CyclicBuffer.
    // The rationale behind this buffer is to share buffer space (SharedMemory file) between two
    // processes. One process produces data, the othere process consumes it. The signalling
    // between the two processes is based on a semaphore (binairy semaphore) The Producer creates
    // the SharedBuffer object, indicting it has the Producer role. It will automatically own
    // the producer lock. If the producer has placed the data in the buffer and would like the
    // consumer to handle it, it signals this by releasing/unlocking the semaphore.
    // The producer has to call the constructor with the administrationSize parameter, and specify an
    // administration area.
    // The consumer, who signals it role during construction, can, if the system is operational,
    // Wait for the Producer lock. Once it holds the producer lock, it can modify/use the SharedBuffer.
    // If the Consumer is finished with the shared buffer, it should signal this by releasing the
    // shared lock again.
    // The consumer construct should be done with
    // TODO use resize from base class
    //
    // MF2018, note: usage of this class is currently not supported when the system time can make large
    //               jumps. Do not use SharedBuffer class when the Time subsystem is not yet available.
    //
    class EXTERNAL SharedBuffer : public DataElementFile {
    private:
        SharedBuffer() = delete;
        SharedBuffer(const SharedBuffer&) = delete;
        SharedBuffer& operator=(const SharedBuffer&) = delete;

    private:
        struct Administration {
            uint32_t _bytesWritten;
        };

    public:
        ~SharedBuffer() override = default;

        // This is the consumer constructor. It should always take place, after, the producer
        // construct. The producer will create the Administration area, and the shared buffer,
        // default size.
        SharedBuffer(const TCHAR name[]);

        // This is the Producer constructor. It sets up all the information and locks the
        // Semaphore by default.
        SharedBuffer(const TCHAR name[], const uint32_t mode, const uint32_t bufferSize, const uint16_t administrationSize);

        inline uint32_t RequestProduce(const uint32_t waitTime)
        {
            uint32_t output = (IsValid() ? _producer.Lock(waitTime) : Core::ERROR_UNAVAILABLE);

            return output;
        }
        inline uint32_t Produced()
        {
            uint32_t output = Core::ERROR_UNAVAILABLE;

            if (IsValid() == true) {
                _administration->_bytesWritten = static_cast<uint32_t>(Size());

                output = _consumer.Unlock();
            }

            return output;
        }

        uint32_t RequestConsume(const uint32_t waitTime)
        {
            uint32_t result = Core::ERROR_UNAVAILABLE;

            if (IsValid() == true) {
                result = _consumer.Lock(waitTime);

                // Realign the buffer with what has been written
                Size(_administration->_bytesWritten);
            }

            return result;
        }

        uint32_t Consumed()
        {
            uint32_t output = Core::ERROR_UNAVAILABLE;

            if (IsValid() == true) {
                _administration->_bytesWritten = static_cast<uint32_t>(Size());

                output = _producer.Unlock();
            }

            return output;
        }

        uint8_t* AdministrationBuffer()
        {
            return _customerAdministration;
        }

        const uint8_t* AdministrationBuffer() const
        {
            return _customerAdministration;
        }

        uint32_t BytesWritten() const
        {
            return (_administration != nullptr ? _administration->_bytesWritten : 0);
        }

        uint32_t User(const string& userName) const
        {
            uint32_t result = (IsValid() ? _administrationBuffer.User(userName) : Core::ERROR_UNAVAILABLE);

            if (result == Core::ERROR_NONE){
                result = DataElementFile::User(userName);
            }

            return result;
        }
        uint32_t Group(const string& groupName) const
        {
            uint32_t result = (IsValid() ? _administrationBuffer.Group(groupName) : Core::ERROR_UNAVAILABLE);

            if (result == Core::ERROR_NONE){
                result = DataElementFile::Group(groupName);
            }

            return result;
        }
        uint32_t Permission(uint16_t mode) const
        {
            uint32_t result = (IsValid() ? _administrationBuffer.Storage().PermissionOnHandle(mode) : Core::ERROR_UNAVAILABLE);

            if (result == Core::ERROR_NONE){
                result = DataElementFile::Permission(mode);
            }

            return result;
        }

    private:
        DataElementFile _administrationBuffer;
        Administration* _administration;
        Core::SharedSemaphore _producer;
        Core::SharedSemaphore _consumer;
        uint8_t* _customerAdministration;
    };
}
} // namespace Thunder::Core

#endif // __SHARED_BUFFER_H
