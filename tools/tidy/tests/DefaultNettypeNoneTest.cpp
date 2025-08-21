//------------------------------------------------------------------------------
// DefaultNettypeNoneTest.cpp
// Test for default_nettype none rules
//
// SPDX-FileCopyrightText: Perrin Tong
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------

#include "Test.h"
#include "TidyFactory.h"
#include "TidyTest.h"

TEST_CASE("DefaultNettypeNone: File with default_nettype none") {
    auto result = runCheckTest("DefaultNettypeNone", R"(`default_nettype none

module top ();
    logic a, b, c;
    assign c = a & b;
endmodule
)");
    CHECK(result);
}

TEST_CASE("DefaultNettypeNone: File without default_nettype none") {
    auto result = runCheckTest("DefaultNettypeNone", R"(
module top ();
    logic a, b, c;
    assign c = a & b;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("DefaultNettypeNone: File with default_nettype wire") {
    auto result = runCheckTest("DefaultNettypeNone", R"(`default_nettype wire

module top ();
    logic a, b, c;
    assign c = a & b;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("DefaultNettypeNone: File with comments before directive") {
    auto result = runCheckTest("DefaultNettypeNone", R"(// Header comment
// Another comment
`default_nettype none

module top ();
    logic a, b, c;
    assign c = a & b;
endmodule
)");
    CHECK(result);
}

TEST_CASE("DefaultNettypeNone: Multiple modules, directive at top") {
    auto result = runCheckTest("DefaultNettypeNone", R"(`default_nettype none

module top ();
    logic a, b, c;
    assign c = a & b;
endmodule

module other ();
    logic x, y, z;
    assign z = x | y;
endmodule
)");
    CHECK(result);
}

TEST_CASE("DefaultNettypeNone: Directive in wrong location") {
    auto result = runCheckTest("DefaultNettypeNone", R"(
module top ();
    logic a, b, c;
    assign c = a & b;
endmodule
)");
    CHECK_FALSE(result);
}
