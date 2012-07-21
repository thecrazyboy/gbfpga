--Legal Notice: (C)2007 Altera Corporation. All rights reserved.  Your
--use of Altera Corporation's design tools, logic functions and other
--software and tools, and its AMPP partner logic functions, and any
--output files any of the foregoing (including device programming or
--simulation files), and any associated documentation or information are
--expressly subject to the terms and conditions of the Altera Program
--License Subscription Agreement or other applicable license agreement,
--including, without limitation, that your use is for the sole purpose
--of programming logic devices manufactured by Altera and sold by Altera
--or its authorized distributors.  Please refer to the applicable
--agreement for further details.


-- turn off superfluous VHDL processor warnings 
-- altera message_level Level1 
-- altera message_off 10034 10035 10036 10037 10230 10240 10030 

library altera;
use altera.altera_europa_support_lib.all;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity custominstruction_cpu_0 is 
        port (
              -- inputs:
                 signal a : IN STD_LOGIC_VECTOR (4 DOWNTO 0);
                 signal c : IN STD_LOGIC_VECTOR (4 DOWNTO 0);
                 signal clk : IN STD_LOGIC;
                 signal clk_en : IN STD_LOGIC;
                 signal dataa : IN STD_LOGIC_VECTOR (31 DOWNTO 0);
                 signal readra : IN STD_LOGIC;
                 signal reset : IN STD_LOGIC;
                 signal start : IN STD_LOGIC;
                 signal writerc : IN STD_LOGIC;

              -- outputs:
                 signal result : OUT STD_LOGIC_VECTOR (31 DOWNTO 0)
              );
end entity custominstruction_cpu_0;


architecture europa of custominstruction_cpu_0 is
component z80acc is 
           port (
                 -- inputs:
                    signal a : IN STD_LOGIC_VECTOR (4 DOWNTO 0);
                    signal c : IN STD_LOGIC_VECTOR (4 DOWNTO 0);
                    signal clk : IN STD_LOGIC;
                    signal clk_en : IN STD_LOGIC;
                    signal dataa : IN STD_LOGIC_VECTOR (31 DOWNTO 0);
                    signal readra : IN STD_LOGIC;
                    signal reset : IN STD_LOGIC;
                    signal start : IN STD_LOGIC;
                    signal writerc : IN STD_LOGIC;

                 -- outputs:
                    signal result : OUT STD_LOGIC_VECTOR (31 DOWNTO 0)
                 );
end component z80acc;

                signal internal_result :  STD_LOGIC_VECTOR (31 DOWNTO 0);

begin

  --s1, which is an e_custom_instruction_slave
  --the_z80acc, which is an e_instance
  the_z80acc : z80acc
    port map(
      result => internal_result,
      a => a,
      c => c,
      clk => clk,
      clk_en => clk_en,
      dataa => dataa,
      readra => readra,
      reset => reset,
      start => start,
      writerc => writerc
    );


  --vhdl renameroo for output signals
  result <= internal_result;

end europa;

