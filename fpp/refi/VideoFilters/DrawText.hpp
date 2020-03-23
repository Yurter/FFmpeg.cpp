#pragma once
#include <string>

namespace fpp::VideoFilter::DrawText {

    struct Text {

        /* The text string to be drawn.
         * The text must be a sequence of UTF-8 encoded characters */
        std::string text;

        /* A text file containing text to be drawn.
         * The text must be a sequence of UTF-8 encoded characters */
        std::string textfile;

        /* The expressions which specify the offsets where text
         * will be drawn within the video frame.
         * They are relative to the top/left border of the output image. */
        std::string x { "0" };
        std::string y { "0" };

    };

    struct Font {

        /* The font family to be used for drawing text */
        std::string font;

        /* The font file to be used for drawing text.
         * The path must be included */
        std::string font_file;

        /* The font size to be used for drawing text */
        std::string size { "16" };

        /* The color to be used for drawing fonts */
        std::string color { "black" };

        /* String which is expanded the same way as text
         * to obtain dynamic fontcolor value */
        std::string color_expr;

        /* Draw the text applying alpha blending.
         * The value can be a number between 0.0 and 1.0 */
        std::string alpha { "1" };

        /* The expressions which specify the offsets where text
         * will be drawn within the video frame */
        std::string x { "0" };
        std::string y { "0" };

        /* The flags to be used for loading the fonts.
         * The flags map the corresponding flags supported by libfreetype,
         * and are a combination of the following values:
         * default
         * no_scale
         * no_hinting
         * render
         * no_bitmap
         * vertical_layout
         * force_autohint
         * crop_bitmap
         * pedantic
         * ignore_global_advance_width
         * no_recurse
         * ignore_transform
         * monochrome
         * linear_design
         * no_autohint */
        std::string load_flags { "default" };

    };

    /* Used to draw a box around text using the background color */
    struct Box {

        /* Used to draw a box around text using the background color.
         * The value must be either 1 (enable) or 0 (disable) */
        std::string enabled { "0" };

        /* The color to be used for drawing box around text */
        std::string color { "white" };

        /* Set the width of the border to be drawn around the box using border_color */
        std::string border_width { "0" };

        /* Set the color to be used for drawing border around text */
        std::string border_color { "black" };

    };

    struct Shadow {

        /* The x and y offsets for the text shadow position
         * with respect to the position of the text.
         * They can be either positive or negative values */
        std::string x { "0" };
        std::string y { "0" };

        /* The color to be used for drawing a shadow behind the drawn text */
        std::string color { "black" };

    };

    struct Time {

        /* Set the initial timecode representation in "hh:mm:ss[:;.]ff" format.
         * It can be used with or without text parameter.
         * timecode_rate option must be specified */
        std::string timecode;

        /* Set the timecode frame rate (timecode only).
         * Value will be rounded to nearest integer. Minimum value is "1".
         * Drop-frame timecode is supported for frame rates 30 & 60 */
        std::string timecode_rate;

        /* If set to 1, the output of the timecode option
         * will wrap around at 24 hours */
        std::string tc24hmax { "0" };

    };

    std::string make(Text text, Font font = Font {}, Box box = Box {});

} // namespace fpp::VideoFilter::DrawText
