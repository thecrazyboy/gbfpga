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

entity seg7_0 is 
        port (
              -- inputs:
                 signal iCLK : IN STD_LOGIC;
                 signal iDIG : IN STD_LOGIC_VECTOR (15 DOWNTO 0);
                 signal iRST_N : IN STD_LOGIC;
                 signal iWR : IN STD_LOGIC;

              -- outputs:
                 signal oSEG0 : OUT STD_LOGIC_VECTOR (6 DOWNTO 0);
                 signal oSEG1 : OUT STD_LOGIC_VECTOR (6 DOWNTO 0);
                 signal oSEG2 : OUT STD_LOGIC_VECTOR (6 DOWNTO 0);
                 signal oSEG3 : OUT STD_LOGIC_VECTOR (6 DOWNTO 0)
              );
end entity seg7_0;


architecture europa of seg7_0 is
component SEG7_LUT_4 is 
           port (
                 -- inputs:
                    signal iCLK : IN STD_LOGIC;
                    signal iDIG : IN STD_LOGIC_VECTOR (15 DOWNTO 0);
                    signal iRST_N : IN STD_LOGIC;
                    signal iWR : IN STD_LOGIC;

                 -- outputs:
                    signal oSEG0 : OUT STD_LOGIC_VECTOR (6 DOWNTO 0);
                    signal oSEG1 : OUT STD_LOGIC_VECTOR (6 DOWNTO 0);
                    signal oSEG2 : OUT STD_LOGIC_VECTOR (6 DOWNTO 0);
                    signal oSEG3 : OUT STD_LOGIC_VECTOR (6 DOWNTO 0)
                 );
end component SEG7_LUT_4;

                signal internal_oSEG0 :  STD_LOGIC_VECTOR (6 DOWNTO 0);
                signal internal_oSEG1 :  STD_LOGIC_VECTOR (6 DOWNTO 0);
                signal internal_oSEG2 :  STD_LOGIC_VECTOR (6 DOWNTO 0);
                signal internal_oSEG3 :  STD_LOGIC_VECTOR (6 DOWNTO 0);

begin

  --the_SEG7_LUT_4, which is an e_instance
  the_SEG7_LUT_4 : SEG7_LUT_4
    port map(
      oSEG0 => internal_oSEG0,
      oSEG1 => internal_oSEG1,
      oSEG2 => internal_oSEG2,
      oSEG3 => internal_oSEG3,
      iCLK => iCLK,
      iDIG => iDIG,
      iRST_N => iRST_N,
      iWR => iWR
    );


  --vhdl renameroo for output signals
  oSEG0 <= internal_oSEG0;
  --vhdl renameroo for output signals
  oSEG1 <= internal_oSEG1;
  --vhdl renameroo for output signals
  oSEG2 <= internal_oSEG2;
  --vhdl renameroo for output signals
  oSEG3 <= internal_oSEG3;

end europa;

