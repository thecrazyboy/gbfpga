library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;

-- Opcodes:
-- 0 : PACK16
-- 1 : Reset DIV
-- 2 : Get DIV
-- 3 : IO tick
-- 4 : Get IF
-- 5 : Set IF
-- 6 : Set TMA
-- 7 : Set TAC
-- 8 : Get TIMA
-- 9 : Set TIMA

entity gbacc is
port
(
   signal clk : in std_logic;
	signal reset : in std_logic;
	signal clk_en : in std_logic;
	signal start : in std_logic;
	signal done : out std_logic;
	signal n : std_logic_vector(3 downto 0);
	signal dataa: in std_logic_vector(31 downto 0);
	signal datab: in std_logic_vector(31 downto 0);
	signal result: out std_logic_vector(31 downto 0)
);
end entity gbacc;

architecture rtl of gbacc is

	signal pack16_result : std_logic_vector(31 downto 0);
	
	signal DIV_r : std_logic_vector(13 downto 0);
	signal TAC_r : std_logic_vector(7 downto 0);
	signal TMA_r : std_logic_vector(7 downto 0);
	
	signal IF_timer_r : std_logic;
	
	signal timer_r : std_logic_vector(15 downto 0);
	
begin
	
	XREG_ST : process(clk, reset)
	variable timer_next : std_logic_vector(16 downto 0);
	begin
		if reset = '1' then
			DIV_r <= (others=>'0');
			TAC_r <= (others=>'0');
			TMA_r <= (others=>'0');
			IF_timer_r <= '0';
			timer_r <= (others=>'0');
		elsif clk'event and clk = '1' and clk_en = '1' and start = '1' then
			case n is
			when "0001" =>	-- Reset DIV
				DIV_r <= (others=>'0');
			when "0011" => -- IO tick
				DIV_r <= DIV_r + 1;
				if TAC_r(2) = '1' then
					timer_next := '0' & timer_r;
					case TAC_r(1 downto 0) is
					when "00" => timer_next := timer_next + 1;
					when "01" => timer_next := timer_next + 64;
					when "10" => timer_next := timer_next + 16;
					when "11" => timer_next := timer_next + 4;
					end case;
					if timer_next(16) = '1' then
					    timer_next(15 downto 8) := TMA_r;
						 IF_timer_r <= '1';
					end if;
					timer_r <= timer_next(15 downto 0);
				end if;
			when "0101" => -- Set IF
				IF_timer_r <= dataa(2);
			when "0110" => -- Set TMA
				TMA_r <= dataa(7 downto 0);
			when "0111" => -- Set TAC
				TAC_r <= dataa(7 downto 0);
			when "1001" => -- Set TMA
				timer_r(15 downto 8) <= dataa(7 downto 0);
			when others =>
			end case;
		end if;
	end process;
	
	result <= 	pack16_result 																	when n = "0000" else
					X"000000" & DIV_r(13 downto 6) 											when n = "0011" else
					X"000000" & dataa(7 downto 3) & If_timer_r & dataa(1 downto 0) when n = "0100" else
					X"000000" & timer_r(15 downto 8)											when n = "1000" else
					(others=>'X');
	
	pack16_result <= X"0000" & 
	          dataa(8)  & dataa(0) & dataa(9)  & dataa(1) & 
	          dataa(10) & dataa(2) & dataa(11) & dataa(3) &
				 dataa(12) & dataa(4) & dataa(13) & dataa(5) &
				 dataa(14) & dataa(6) & dataa(15) & dataa(7);
				 
	done <= start;
	
end architecture;