// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT

#include "Test.h"
#include "TidyFactory.h"
#include "TidyTest.h"

TEST_CASE("LineFormatting: Valid line lengths") {
    auto result = runCheckTest("LineFormatting", R"(
module top ();
    logic a, b, c;
    assign c = a & b;
endmodule
)");
    CHECK(result);
}

TEST_CASE("LineFormatting: Line too long") {
    auto result = runCheckTest("LineFormatting", R"(
module top ();
    logic very_long_signal_name_that_exceeds_eighty_characters_and_should_trigger_error;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("LineFormatting: Tab characters") {
    auto result = runCheckTest("LineFormatting", R"(
module top ();
	logic a, b, c;  // This line contains a tab character
    assign c = a & b;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("LineFormatting: Exactly 80 characters") {
    auto result = runCheckTest("LineFormatting", R"(
module top ();
    logic signal_name_that_is_exactly_eighty_characters_long_but_not_over;
endmodule
)");
    CHECK(result);
}

TEST_CASE("LineFormatting: Multiple violations") {
    auto result = runCheckTest("LineFormatting", R"(
module top ();
	logic a;  // Tab character
    logic very_long_signal_name_that_definitely_exceeds_the_eighty_character_limit_rule;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("LineFormatting: No violations") {
    auto result = runCheckTest("LineFormatting", R"(
module top ();
    logic a, b, c, d, e;
    
    always_comb begin
        c = a & b;
        d = a | b;
        e = a ^ b;
    end
endmodule
)");
    CHECK(result);
}