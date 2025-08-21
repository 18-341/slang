// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT

#include "Test.h"
#include "TidyFactory.h"
#include "TidyTest.h"

TEST_CASE("NoVerilogSyntax: Modern SystemVerilog syntax - valid") {
    auto result = runCheckTest("NoVerilogSyntax", R"(
module top (
    input logic clk,
    input logic rst_n,
    output logic [7:0] data_out
);
    logic [7:0] internal_reg;
    logic enable;
    int counter;
    
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            internal_reg <= 8'h0;
        else if (enable)
            internal_reg <= internal_reg + 1;
    end
    
    always_comb begin
        data_out = internal_reg;
    end
endmodule
)");
    CHECK(result);
}

TEST_CASE("NoVerilogSyntax: Deprecated wire keyword") {
    auto result = runCheckTest("NoVerilogSyntax", R"(
module top ();
    wire [7:0] old_signal;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("NoVerilogSyntax: Deprecated reg keyword") {
    auto result = runCheckTest("NoVerilogSyntax", R"(
module top ();
    reg [7:0] old_register;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("NoVerilogSyntax: Deprecated integer keyword") {
    auto result = runCheckTest("NoVerilogSyntax", R"(
module top ();
    integer old_int;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("NoVerilogSyntax: Deprecated always block") {
    auto result = runCheckTest("NoVerilogSyntax", R"(
module top (
    input logic clk,
    output logic [7:0] data
);
    logic [7:0] counter;
    
    always @(posedge clk) begin
        counter <= counter + 1;
    end
    
    assign data = counter;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("NoVerilogSyntax: Non-ANSI port style") {
    auto result = runCheckTest("NoVerilogSyntax", R"(
module top (clk, rst_n, data_out);
    input clk;
    input rst_n;
    output [7:0] data_out;
    
    logic [7:0] internal_data;
    
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            internal_data <= 8'h0;
        else
            internal_data <= internal_data + 1;
    end
    
    assign data_out = internal_data;
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("NoVerilogSyntax: Multiple deprecated constructs") {
    auto result = runCheckTest("NoVerilogSyntax", R"(
module top (clk, data_out);
    input clk;
    output [7:0] data_out;
    
    wire [7:0] bus_signal;
    reg [7:0] register_signal;
    integer count_var;
    
    always @(posedge clk) begin
        register_signal <= register_signal + 1;
    end
    
    assign bus_signal = register_signal;
    assign data_out = bus_signal;
endmodule
)");
    CHECK_FALSE(result);
}