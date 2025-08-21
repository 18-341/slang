// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT

#include "Test.h"
#include "TidyFactory.h"
#include "TidyTest.h"

TEST_CASE("ConstantFormatting: Valid constants - 0 and 1 allowed") {
    auto result = runCheckTest("ConstantFormatting", R"(
module top ();
    logic [7:0] data;
    logic enable;
    
    always_comb begin
        enable = 1;
        data = 0;
    end
endmodule
)");
    CHECK(result);
}

TEST_CASE("ConstantFormatting: Valid sized constants") {
    auto result = runCheckTest("ConstantFormatting", R"(
module top ();
    logic [7:0] data = 8'hFF;
    logic [3:0] nibble = 4'b1010;
    logic [15:0] word = 16'd65535;
    logic [2:0] octal_val = 3'o7;
endmodule
)");
    CHECK(result);
}

TEST_CASE("ConstantFormatting: Valid binary with underscores") {
    auto result = runCheckTest("ConstantFormatting", R"(
module top ();
    logic [7:0] data = 8'b1010_1100;
    logic [15:0] word = 16'b1111_0000_1010_0101;
    logic [11:0] wide = 12'b1100_0011_1111;
endmodule
)");
    CHECK(result);
}

TEST_CASE("ConstantFormatting: Invalid unsized decimal constants") {
    auto result = runCheckTest("ConstantFormatting", R"(
module top ();
    logic [7:0] data = 255;
    logic [3:0] nibble = 15;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("ConstantFormatting: Invalid unsized based constants") {
    auto result = runCheckTest("ConstantFormatting", R"(
module top ();
    logic [7:0] data = 'hFF;
    logic [3:0] nibble = 'b1010;
    logic [2:0] octal = 'o7;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("ConstantFormatting: Invalid binary without underscores") {
    auto result = runCheckTest("ConstantFormatting", R"(
module top ();
    logic [7:0] data = 8'b10101100;
    logic [15:0] word = 16'b1111000010100101;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("ConstantFormatting: Exclusions - bit selects allowed") {
    auto result = runCheckTest("ConstantFormatting", R"(
module top ();
    logic [7:0] bus;
    logic [15:0] data;
    logic bit_out;
    
    always_comb begin
        bit_out = bus[3];
        data = bus[7:0];
    end
endmodule
)");
    CHECK(result);
}

TEST_CASE("ConstantFormatting: Exclusions - parameter assignments allowed") {
    auto result = runCheckTest("ConstantFormatting", R"(
module top #(
    parameter WIDTH = 8,
    parameter DEPTH = 256
) ();
    logic [WIDTH-1:0] data [0:DEPTH-1];
endmodule
)");
    CHECK(result);
}

TEST_CASE("ConstantFormatting: Exclusions - array dimensions allowed") {
    auto result = runCheckTest("ConstantFormatting", R"(
module top ();
    logic [7:0] memory [0:255];
    logic [3:0] small_array [0:15];
endmodule
)");
    CHECK(result);
}

TEST_CASE("ConstantFormatting: Exclusions - generate loop bounds allowed") {
    auto result = runCheckTest("ConstantFormatting", R"(
module top ();
    genvar i;
    generate
        for (i = 0; i < 8; i++) begin : gen_loop
            logic [7:0] data;
        end
    endgenerate
endmodule
)");
    CHECK(result);
}