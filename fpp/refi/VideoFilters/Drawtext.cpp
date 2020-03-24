#include "DrawText.hpp"

namespace fpp::VideoFilter::DrawText {

    std::string make(Text text, Font font, Box box, Shadow shadow, Time time) {

        std::string filter_description {
            "drawtext="
        };
        const std::string param_delim { ": " };
        const auto create_param {
            [param_delim](const std::string& key, const std::string& value) {
                return key + "=" + value + param_delim;
            }
        };
        const auto append_param {
            [&filter_description,&create_param]
            (const std::string& key, const std::string& value) {
                filter_description.append(create_param(key, value));
            }
        };
        const auto trim_last_delim {
            [param_delim](std::string& str) {
                if (const auto pos {
                    str.rfind(param_delim)
                }; pos != std::string::npos) {
                    str.erase(pos, param_delim.length());
                }
            }
        };

        #define param_setted(var,member) var.member.empty()
        #define make_param_pair(var,member) #member, var.member

        #define parse_param(var,member)\
            do {\
                if (!param_setted(var,member)) {\
                    append_param(make_param_pair(var,member));\
                }\
            } while (false)

        // Text
        parse_param(text, text);
        parse_param(text, textfile);
        parse_param(text, x);
        parse_param(text, y);

        // Font
        parse_param(font, font);
        parse_param(font, fontfile);
        parse_param(font, fontsize);
        parse_param(font, fontcolor);
        parse_param(font, fontcolor_expr);
        parse_param(font, alpha);
        parse_param(font, ft_load_flags);

        //Box
        parse_param(box, box);
        parse_param(box, boxcolor);
        parse_param(box, borderw);

        //Shadow
        parse_param(shadow, shadowcolor);
        parse_param(shadow, shadowx);
        parse_param(shadow, shadowy);

        //Time
        parse_param(time, timecode);
        parse_param(time, timecode_rate);
        parse_param(time, tc24hmax);

        #undef param_setted
        #undef make_param_pair
        #undef parse_param

        trim_last_delim(filter_description);

        return filter_description;

    }

} // namespace fpp::VideoFilter::DrawText
