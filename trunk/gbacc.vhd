library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;

entity gbacc is
port
(
	signal dataa: in std_logic_vector(31 downto 0);
	signal datab: in std_logic_vector(31 downto 0);
	signal result: out std_logic_vector(31 downto 0)
);
end entity gbacc;

architecture rtl of gbacc is
begin
	
	result <= X"0000" & 
	          dataa(8)  & dataa(0) & dataa(9)  & dataa(1) & 
	          dataa(10) & dataa(2) & dataa(11) & dataa(3) &
				 dataa(12) & dataa(4) & dataa(13) & dataa(5) &
				 dataa(14) & dataa(6) & dataa(15) & dataa(7);
	
end architecture;