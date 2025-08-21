//------------------------------------------------------------------------------
// ConstantWidthCheckTest.cpp
// Test for constant width checking rules
//
// SPDX-FileCopyrightText: Perrin Tong
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------

#include "Test.h"
#include "TidyFactory.h"
#include "TidyTest.h"

TEST_CASE("ConstantWidthCheck: Valid constants") {
    auto result = runCheckTest("ConstantWidthCheck", R"(
module top ();
    logic [3:0] a = 4'hF;     // Valid: 15 fits in 4 bits
    logic [2:0] b = 3'b101;   // Valid: 5 fits in 3 bits  
    logic [3:0] c = 4'd10;    // Valid: 10 fits in 4 bits
    logic [2:0] d = 3'o7;     // Valid: 7 fits in 3 bits
endmodule
)");
    CHECK(result);
}

TEST_CASE("ConstantWidthCheck: Binary overflow") {
    auto result = runCheckTest("ConstantWidthCheck", R"(
module top ();
    logic [2:0] a = 3'b1000;  // Invalid: 8 doesn't fit in 3 bits
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("ConstantWidthCheck: Decimal overflow") {
    auto result = runCheckTest("ConstantWidthCheck", R"(
module top ();
    logic [3:0] a = 4'd16;    // Invalid: 16 doesn't fit in 4 bits
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("ConstantWidthCheck: Hexadecimal overflow") {
    auto result = runCheckTest("ConstantWidthCheck", R"(
module top ();
    logic [3:0] a = 4'h10;    // Invalid: 16 doesn't fit in 4 bits
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("ConstantWidthCheck: Octal overflow") {
    auto result = runCheckTest("ConstantWidthCheck", R"(
module top ();
    logic [2:0] a = 3'o10;    // Invalid: 8 doesn't fit in 3 bits
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("ConstantWidthCheck: Edge cases") {
    auto result = runCheckTest("ConstantWidthCheck", R"(
module top ();
    logic [3:0] a = 4'hF;     // Valid: exactly 15 (max for 4 bits)
    logic [3:0] b = 4'd15;    // Valid: exactly 15 (max for 4 bits)
    logic [0:0] c = 1'b1;     // Valid: 1 bit
endmodule
)");
    CHECK(result);
}

TEST_CASE("ConstantWidthCheck: Multiple violations") {
    auto result = runCheckTest("ConstantWidthCheck", R"(
module top ();
    logic [2:0] a = 3'd8;     // Invalid: 8 doesn't fit in 3 bits
    logic [1:0] b = 2'd4;     // Invalid: 4 doesn't fit in 2 bits
endmodule
)");
    CHECK_FALSE(result);
}