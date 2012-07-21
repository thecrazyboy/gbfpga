-- VHDL Custom Instruction Template File for Internal Register Logic
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;

entity z80acc is
port(
	signal clk: in std_logic;-- CPU system clock (required for multi-cycle or extended multi-cycle)
	signal reset: in std_logic;-- CPU master asynchronous active high reset (required for multi-cycle or extended multi-cycle)
	signal clk_en: in std_logic;-- Clock-qualifier (required for multi-cycle or extended multi-cycle)
	signal start: in std_logic;-- Active high signal used to specify that inputs are valid (required for multi-cycle or extended multi-cycle)
	signal done: out std_logic;-- Active high signal used to notify the CPU that result is valid 	(required for variable multi-cycle or extended variable multi-cycle)
	--signal n: in std_logic_vector(7 downto 0);-- N-field selector (required for extended), modify width to match the number of unique operations within the custom instruction
	signal dataa: in std_logic_vector(31 downto 0);-- Operand A (always required)
	--signal datab: in std_logic_vector(31 downto 0);-- Operand B (optional)
	signal a: in std_logic_vector(4 downto 0);-- Internal operand A index register
	--signal b: in std_logic_vector(4 downto 0);-- Internal operand B index register
	signal c: in std_logic_vector(4 downto 0);-- Internal result index register
	signal readra: in std_logic;-- Read operand A from CPU (otherwise use internal operand A)
	--signal readrb: in std_logic;-- Read operand B from CPU (otherwise use internal operand B)
	signal writerc: in std_logic;-- Write result to CPU (otherwise write to internal result)
	signal result: out std_logic_vector(31 downto 0)-- result (always required)
);
end entity z80acc;

architecture a_z80acc of z80acc is
	-- local custom instruction signals
	type t_reg_array is array(integer range 7 downto 0) of std_logic_vector(7 downto 0);
	signal reg_r : t_reg_array;
	signal reg_next : t_reg_array;
	signal result_next : std_logic_vector(7 downto 0);
	
	constant OP_A : integer := 0;
	constant OP_F : integer := 1;
	constant OP_B : integer := 2;
	constant OP_C : integer := 3;
	constant OP_D : integer := 4;
	constant OP_E : integer := 5;
	constant OP_H : integer := 6;
	constant OP_L : integer := 7;
	constant OP_AF : integer := 8;
	constant OP_BC : integer := 9;
	constant OP_DE : integer := 10;
	constant OP_HL : integer := 11;

begin
	-- custom instruction logic (note: external interfaces can be used as well)
	-- use the n[7..0] port as a select signal on a multiplexer to select the value to feed result[31..0]
	XFLIP : process(clk, reset)
	begin
		if (reset = '1') then
			for i in 7 downto 0 loop
				reg_r(i) <= (others=>'0');
			end loop;
		elsif (clk'event and clk = '1') then
			reg_r <= reg_next;
		end if;
	end process;
	
	XRESULT : process(clk_en, start, dataa, reg_r, a, readra)
	begin
		if (clk_en = '1' and start = '1') then
			if (readra = '1') then
				result_next <= dataa(7 downto 0);
			else
				result_next <= reg_r(conv_integer(unsigned(a(2 downto 0))));
			end if;
		end if;
	end process;
	
	XSTORE : process(reg_r, clk_en, start, writerc, c, result_next)
	begin
		reg_next <= reg_r;
		if (clk_en = '1' and start = '1') then
			if (writerc = '0') then
				reg_next(conv_integer(unsigned(c(2 downto 0)))) <= result_next;
			end if;
		end if;
	end process;
	
	result <= "000000000000000000000000" & result_next;

end architecture a_z80acc;