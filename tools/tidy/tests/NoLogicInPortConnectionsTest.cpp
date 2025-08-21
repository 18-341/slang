//------------------------------------------------------------------------------
// NoLogicInPortConnectionsTest.cpp
// Test for no logic in port connections rules
//
// SPDX-FileCopyrightText: Perrin Tong
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------

#include "Test.h"
#include "TidyFactory.h"
#include "TidyTest.h"

TEST_CASE("NoLogicInPortConnections: Valid port connections") {
    auto result = runCheckTest("NoLogicInPortConnections", R"(
module multiplexer(
    input logic sel,
    input logic I0, I1,
    output logic out
);
    assign out = sel ? I1 : I0;
endmodule

module top();
    logic a, b, c, d;
    
    multiplexer mux(
        .sel(a),
        .I0(b),
        .I1(c),
        .out(d)
    );
endmodule
)");
    CHECK(result);
}

TEST_CASE("NoLogicInPortConnections: Logic in port connections") {
    auto result = runCheckTest("NoLogicInPortConnections", R"(
module multiplexer(
    input logic sel,
    input logic I0, I1,
    output logic out
);
    assign out = sel ? I1 : I0;
endmodule

module top();
    logic a, b, c, d;
    
    multiplexer mux(
        .sel(a),
        .I0(~a|b),
        .I1(c),
        .out(~d)
    );
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("NoLogicInPortConnections: Range selects allowed") {
    auto result = runCheckTest("NoLogicInPortConnections", R"(
module test_module(
    input logic [3:0] data_in,
    output logic [3:0] data_out
);
    assign data_out = data_in;
endmodule

module top();
    logic [7:0] bus;
    logic [3:0] result;
    
    test_module tm(
        .data_in(bus[3:0]),
        .data_out(result)
    );
endmodule
)");
    CHECK(result);
}

TEST_CASE("NoLogicInPortConnections: Element selects allowed") {
    auto result = runCheckTest("NoLogicInPortConnections", R"(
module test_module(
    input logic data_in,
    output logic bit_out
);
    assign bit_out = data_in;
endmodule

module top();
    logic [7:0] bus;
    logic bit;
    
    test_module tm(
        .data_in(bus[0]),
        .bit_out(bit)
    );
endmodule
)");
    CHECK(result);
}

TEST_CASE("NoLogicInPortConnections: Concatenations allowed") {
    auto result = runCheckTest("NoLogicInPortConnections", R"(
module test_module(
    input logic [7:0] data_in,
    output logic [7:0] data_out
);
    assign data_out = data_in;
endmodule

module top();
    logic [3:0] a, b;
    logic [7:0] result;
    
    test_module tm(
        .data_in({a, b}),
        .data_out(result)
    );
endmodule
)");
    CHECK(result);
}

TEST_CASE("NoLogicInPortConnections: Constants allowed") {
    auto result = runCheckTest("NoLogicInPortConnections", R"(
module test_module(
    input logic [3:0] data_in,
    input logic enable,
    output logic [3:0] data_out
);
    assign data_out = enable ? data_in : 4'b0;
endmodule

module top();
    logic [3:0] data, result;
    
    test_module tm(
        .data_in(data),
        .enable(1'b1),
        .data_out(result)
    );
endmodule
)");
    CHECK(result);
}