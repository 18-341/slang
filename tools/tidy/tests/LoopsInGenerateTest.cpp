//------------------------------------------------------------------------------
// LoopsInGenerateTest.cpp
// Test for loops in generate blocks
//
// SPDX-FileCopyrightText: Perrin Tong
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------

#include "Test.h"
#include "TidyFactory.h"
#include "TidyTest.h"

TEST_CASE("LoopsInGenerate: For loop inside generate") {
    auto result = runCheckTest("LoopsInGenerate", R"(
module top ();
    genvar i;
    generate
        for (i = 0; i < 4; i++) begin : gen_loop
            logic signal;
        end
    endgenerate
endmodule
)");
    CHECK(result);
}

TEST_CASE("LoopsInGenerate: For loop outside generate - module level") {
    auto result = runCheckTest("LoopsInGenerate", R"(
module top ();
    genvar i;
    for (i = 0; i < 4; i++) begin : bad_loop
        logic signal;
    end
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("LoopsInGenerate: For loop in always block is OK") {
    auto result = runCheckTest("LoopsInGenerate", R"(
module top ();
    logic [3:0] data [0:3];
    logic [3:0] sum;
    
    always_comb begin
        sum = 0;
        for (int i = 0; i < 4; i++) begin
            sum = sum + data[i];
        end
    end
endmodule
)");
    CHECK(result);
}

TEST_CASE("LoopsInGenerate: Multiple for loops in generate") {
    auto result = runCheckTest("LoopsInGenerate", R"(
module top ();
    genvar i, j;
    generate
        for (i = 0; i < 4; i++) begin : gen_loop1
            for (j = 0; j < 2; j++) begin : gen_loop2
                logic signal;
            end
        end
    endgenerate
endmodule
)");
    CHECK(result);
}

TEST_CASE("LoopsInGenerate: Mix of valid and invalid loops") {
    auto result = runCheckTest("LoopsInGenerate", R"(
module top ();
    genvar i, j;
    
    generate
        for (i = 0; i < 4; i++) begin : good_loop
            logic signal;
        end
    endgenerate
    
    for (j = 0; j < 2; j++) begin : bad_loop
        logic other_signal;
    end
endmodule
)");
    CHECK_FALSE(result);
}