#include "DrawText.hpp"

namespace fpp::VideoFilter::DrawText {

    //R"(drawtext=fontfile=/path/to/font.ttf: text='Stack Overflow': fontcolor=white: fontsize=24: box=1: boxcolor=black@0.5: boxborderw=5: x=(w-text_w)/2: y=(h-text_h)/2)"
    //drawtext=text='Stack Overflow': fontcolor=white: fontsize=24: box=1: boxcolor=black@0.5: boxborderw=5: x=(w-text_w)/2: y=(h-text_h)/2
    //drawtext=text='Hello World': x=100: y=500:
    std::string make(VideoFilter::DrawText::Text text
                    , VideoFilter::DrawText::Font font
                    , VideoFilter::DrawText::Box box
                    ) {
        std::string filter_description {
            "drawtext="
        };
        const auto wrap_if_has_spaces {
            [](std::string& value) {
                if (const auto pos { value.find(' ') }; pos != std::string::npos) {
                    value = "'" + value + "'";
                }
            }
        };
        const auto create_param {
            [wrap_if_has_spaces](const std::string& key, std::string value) {
                wrap_if_has_spaces(value);
                return key + "=" + value + ": ";
            }
        };
        const auto append_param {
            [&filter_description,&create_param]
            (const std::string& key, const std::string& value) {
                filter_description.append(create_param(key, value));
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

        return filter_description;
    }

} // namespace fpp::VideoFilter::DrawText
