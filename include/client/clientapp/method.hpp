#ifndef STAGDEER_METHOED
#define STAGDEER_METHOED

#include <string>
namespace stagdeer {
    enum class httpMethod {
        GET , POST
    };

    struct methodTool {

        static std::string methodToStr(stagdeer::httpMethod method__) {
            switch (method__) {
                case stagdeer::httpMethod::GET : {return "GET"; break;}
                case stagdeer::httpMethod::POST : {return "POST"; break;}
                default: {return "GET"; break;};
            };
        }

    };
}

#endif