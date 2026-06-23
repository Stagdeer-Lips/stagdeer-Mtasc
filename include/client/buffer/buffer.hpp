#ifndef STAGDEER_CLIENT_BUFFER
#define STAGDEER_CLIENT_BUFFER

#include <cstddef>
#include <cstring>
#include <utility>
#include <vector>

namespace stagdeer {
    namespace client {
        class readBuffer {

            public:
            
            readBuffer() = default;
            ~readBuffer() = default;

            readBuffer(readBuffer&& buffer_Other)
            noexcept: 
                M_buffer_(std::move(buffer_Other.M_buffer_)),
                M_read_pointer(std::move(buffer_Other.M_read_pointer)),
                M_write_pointer(std::move(buffer_Other.M_write_pointer)) {};
                readBuffer& operator=(readBuffer&& buffer_Other) {
                    if (this != &buffer_Other) {
                        M_buffer_ = std::move(buffer_Other.M_buffer_);
                        M_read_pointer = std::move(buffer_Other.M_read_pointer);
                        M_write_pointer = std::move(buffer_Other.M_write_pointer);
                    }
                    return *this;
                };

            readBuffer(const readBuffer&) = default;
            readBuffer& operator=(readBuffer&) = default;

            void appendTobuffer(const char* M_data__ , size_t M_data_size__) {
                M_ensureWriteableBytes(M_data_size__);
                memcpy(M_buffer_.data() + M_write_pointer , M_data__ , M_data_size__);
                M_write_pointer += M_data_size__;
            }

            void retrieveData(size_t M_data_len__) {
                M_read_pointer += M_data_len__;
                if (M_read_pointer == M_write_pointer) {
                    M_read_pointer = M_write_pointer = 0;
                }
            }

            const char* peekData() const {
                return M_buffer_.data() + M_read_pointer;
            }

            size_t readableBytes() const {
                return M_write_pointer - M_read_pointer;
            }

            private:

            void M_ensureWriteableBytes(size_t M_data_len) {
                if (M_buffer_.size() - M_write_pointer >= M_data_len) {
                    return;
                } else if (M_read_pointer + (M_buffer_.size() - M_write_pointer) >= M_data_len) {
                    std::memmove(M_buffer_.data() , M_buffer_.data() 
                        + M_read_pointer , readableBytes());
                        M_write_pointer = readableBytes();
                        M_read_pointer = 0;
                } else {
                    M_buffer_.resize(M_write_pointer + M_data_len);
                }
            }

            std::vector<char> M_buffer_;
            size_t M_read_pointer;
            size_t M_write_pointer;
        };
    }
}

#endif
