#ifndef STAGDEER_OPERATOR_UTIL
#define STAGDEER_OPERATOR_UTIL

#include <type_traits>
#include <utility>

namespace stagdeer {
    namespace util {

        template<typename Tp , typename Up>
        auto operator$(Tp&& __M_VALUE, Up&& __M_METHOD) {
            return __M_METHOD(std::forward<Tp>(__M_VALUE));
        }

        template<typename Tp , typename Up>
        auto operator|(Tp&& __M_VALUE , Up&& __M_METHOD) {
            return __M_METHOD(std::forward<Tp>(__M_VALUE));
        }

        namespace lamdba_trais {
            template<bool Tp>
            struct constraint {
                using type = int;
            };

            template<>
            struct constraint<false> {};

            template<typename Tp , typename ... Args>
            struct M_get_lamdba_ret_Tp {
                using __M_ret_lmdba = std::invoke_result_t<Tp, Args ...>;
            };

            template<typename Tp , typename Up>
            struct M_is_retTp {
                static constexpr bool __is_M_ret_Tp = std::is_same_v<Tp, Up>;
            };
        }
    }
}

#endif